// STD
#include <algorithm>
#include <concepts>
#include <iomanip>
#include <random>
#include <set>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Meta/for.hpp>
#include <Engine/Math/math.hpp>
#include <Engine/ArrayView.hpp>
#include <Engine/Noise/noise.hpp>

// Game
#include <Game/systems/EntityNetworkingSystem.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/systems/ZoneManagementSystem.hpp>
#include <Game/systems/all.hpp>

// TODO: Do we really need all these components? Maybe try to clean this up.
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/MapAreaComponent.hpp>
#include <Game/comps/NetworkComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/comps/SpriteComponent.hpp>

namespace {
	using namespace Game;
	using PlayerFilter = Engine::ECS::EntityFilterList<
		PlayerFlag
	>;

	constexpr uint8 msgPadSeq[] = {0x54, 0x68, 0x65, 0x20, 0x63, 0x61, 0x6B, 0x65, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x6C, 0x69, 0x65, 0x2E, 0x20};

	void writeMessagePadding(Engine::Net::BufferWriter& msg) {
		while (msg.write(msgPadSeq)) {}
		msg.write(msgPadSeq, msg.space());
	}

	bool verifyMessagePadding(Engine::Net::BufferReader& msg) {
		if (msg.size() != sizeof(Engine::Net::Packet::body)) { return false; }
		
		constexpr auto psz = sizeof(msgPadSeq);
		auto cur = msg.peek();
		const auto end = msg.end() - psz;
		auto len = end - cur;

		while (len > 0) {
			if (memcmp(cur, msgPadSeq, len < psz ? len : psz) != 0) {
				return false;
			}
			cur += psz;
			len = end - cur;
		}

		return true;
	}
}
#if DEBUG
namespace {
	struct HandleMessageDef_DebugBreak_Struct {
		Engine::Net::BufferReader& msg;
		~HandleMessageDef_DebugBreak_Struct() {
			ENGINE_DEBUG_ASSERT(msg.remaining() == 0, "Incomplete read of message.");
		}
	};
}
#define HandleMessageDef_DebugBreak(MsgType) HandleMessageDef_DebugBreak_Struct _ENGINE_temp_object_for_HandleMessageDef_DebugBreak_Struct{msg}
#else
#define HandleMessageDef_DebugBreak(...)
#endif


#define HandleMessageDef(MsgType) \
	template<> \
	void NetworkingSystem::handleMessageType<MsgType>(EngineInstance& engine, ConnectionInfo& from, const Engine::Net::MessageHeader head, Engine::Net::BufferReader& msg) { \
	HandleMessageDef_DebugBreak(#MsgType);


namespace Game {
	HandleMessageDef(MessageType::UNKNOWN)
	}

	HandleMessageDef(MessageType::DISCOVER_SERVER)
		// TODO: rate limit per ip (longer if invalid packet)
		if (verifyMessagePadding(msg)) {
			if (auto reply = from.beginMessage<MessageType::SERVER_INFO>()) {
				//constexpr char name[] = "This is the name of the server";
				std::string name = "This is the name of the server ";
				name += std::to_string(Engine::getGlobalConfig().port);
				reply.write<int>(int(std::size(name)));
				reply.write(name.data(), std::size(name));
			}
		} else if constexpr (ENGINE_DEBUG) {
			[[maybe_unused]] const auto rem = msg.remaining();
			ENGINE_DEBUG_ASSERT(false);
		}

		msg.discard();
	}
	
	HandleMessageDef(MessageType::SERVER_INFO)
		#if ENGINE_CLIENT
			int len;
			if (!msg.read<int>(&len)) { return; }

			auto& world = engine.getWorld();
			auto& netSys = world.getSystem<NetworkingSystem>();
			const char* name = reinterpret_cast<const char*>(msg.read(len));
			auto& servInfo = netSys.servers[from.address()];
			servInfo.name.assign(name, len);
			servInfo.lastUpdate = world.getTime();
		#endif
	}

	HandleMessageDef(MessageType::CONNECT_REQUEST)
		uint16 remote = {};
		msg.read<uint16>(&remote);

		if (!verifyMessagePadding(msg)) {
			// TODO: rate limit connections with invalid messages
			ENGINE_WARN("Got invalid connection request from ", from.address());
			return;
		} else {
			msg.discard();
		}

		if (from.getKeyRemote() && from.getKeyRemote() != remote) {
			ENGINE_WARN("Got connection request with invalid key from ", from.address(), " (", remote, " != ", from.getKeyRemote(), ")");
			return msg.discard();
		} else {
			from.setKeyRemote(remote);
		}

		if (from.getState() == ConnectionState::Disconnected) {
			ENGINE_DEBUG_ASSERT(from.getKeyLocal() == 0, "It should not be possible to have a local key in an unconnected state. This is a bug.");
			from.setState(ConnectionState::Connecting);

			auto& world = engine.getWorld();
			auto& netSys = world.getSystem<NetworkingSystem>();
			from.setKeyLocal(netSys.genKey());
		} else {
			ENGINE_DEBUG_ASSERT(from.getKeyLocal() != 0, "It should not be possible to be missing a local key in any non-unconnected state. This is a bug.");
		}

		if (auto reply = from.beginMessage<MessageType::CONNECT_CHALLENGE>()) {
			reply.write(from.getKeyLocal());
			ENGINE_LOG("CONNECT_REQUEST from ", from.address(), " lkey: ", from.getKeyLocal(), " rkey: ", from.getKeyRemote());
		}
	}
	
	HandleMessageDef(MessageType::CONNECT_CHALLENGE)
		if (from.getKeyRemote()) {
			ENGINE_WARN("Extraneous CONNECT_CHALLENGE received. Ignoring.");
			ENGINE_DEBUG_ASSERT(from.getState() == ConnectionState::Connected);
			return msg.discard();
		}

		uint16 remote = {};
		if (!msg.read<uint16>(&remote)) {
			ENGINE_WARN("Invalid connection challenge received.");
			return msg.discard();
		}

		if (auto reply = from.beginMessage<MessageType::CONNECT_CONFIRM>()) {
			reply.write(remote);
			writeMessagePadding(reply.getBufferWriter());
			from.setKeyRemote(remote);
			from.setState(ConnectionState::Connected);
			ENGINE_LOG("CONNECT_CHALLENGE from ", from.address(), " - ", &from, " lkey: ", from.getKeyLocal(), " rkey: ", from.getKeyRemote());
		} else [[unlikely]] {
			ENGINE_WARN("Unable to send CONNECT_CONFIRM");
		}
	}

	HandleMessageDef(MessageType::CONNECT_CONFIRM)
		uint16 key = {};
		msg.read<uint16>(&key);

		if (key != from.getKeyLocal()) {
			ENGINE_WARN("CONNECT_CONFIRM: Invalid recv key ", key, " != ", from.getKeyLocal());
			return msg.discard();
		}

		if (!verifyMessagePadding(msg)) {
			// TODO: rate limit connections with invalid messages
			ENGINE_WARN("Got invalid connection confirm from ", from.address());
			return;
		} else {
			msg.discard();
		}

		auto& world = engine.getWorld();
		auto& netSys = world.getSystem<NetworkingSystem>();

		from.setState(ConnectionState::Connected);
		netSys.addPlayer(from); // TODO: this step should probably be delayed until we get some kind of CONFIG_CONFIRM message.
		ENGINE_LOG("CONNECT_CONFIRM from ", from.address(), " - ", &from, " lkey: ", from.getKeyLocal(), " rkey: ", from.getKeyRemote(), " tick: ", world.getTick(), " ", from.ent);


		// TODO: change message type of this (for client). This isnt a confirmation this is initial sync or similar.
		if (auto reply = from.beginMessage<MessageType::ECS_INIT>()) {
			ENGINE_DEBUG_ASSERT(from.ent, "Attempting to network invalid entity.");
			reply.write(from.ent);
			reply.write(world.getTick());
		}

		if (auto reply = from.beginMessage<MessageType::CONFIG_NETWORK>()) {
			reply.write(from.getPacketRecvRate());
		}
	}

	HandleMessageDef(MessageType::DISCONNECT)
		ENGINE_LOG("MessageType::DISCONNECT ", from.address());
		auto& world = engine.getWorld();
		auto& netSys = world.getSystem<NetworkingSystem>();
		netSys.disconnect(from);
	}

	HandleMessageDef(MessageType::CONFIG_NETWORK)
		float32 rate;
		if (!msg.read(&rate)) {
			ENGINE_WARN("Invalid CONFIG_NETWORK received.");
			return;
		}

		if constexpr (ENGINE_CLIENT) {
			if (auto reply = from.beginMessage<MessageType::CONFIG_NETWORK>()) {
				reply.write(from.getPacketRecvRate());
			}
		}

		const float32 maxSendRate = Engine::getGlobalConfig().cvars.net_packet_rate_max;
		const float32 minSendRate = Engine::getGlobalConfig().cvars.net_packet_rate_min;

		// We need this check because MSVC does not handle comparisons correctly for non-finite values even when is_iec559 is true.
		if (!std::isfinite(rate)) {
			rate = minSendRate;
		}

		const auto rateClamped = std::max(minSendRate, std::min(rate, maxSendRate));
		ENGINE_LOG("Network send rate updated: ", rateClamped, " (", rate, ")");
		from.setPacketSendRate(rateClamped);
	}
}
#undef HandleMessageDef

namespace Game {
	namespace Net = Engine::Net;
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, group{Engine::getGlobalConfig().group}
		, socket{Engine::getGlobalConfig().port, Engine::Net::SocketFlag::NonBlocking}
		#if ENGINE_SERVER
		, discoverServerSocket{Net::UDPSocket::doNotInitialize}
		#endif
		, rng{pcg_extras::seed_seq_from<std::random_device>{}} {

		ENGINE_LOG("Listening on port ", socket.getAddress().port);

		setMessageHandler(MessageType::UNKNOWN, handleMessageType<MessageType::UNKNOWN>);
		setMessageHandler(MessageType::DISCOVER_SERVER, handleMessageType<MessageType::DISCOVER_SERVER>);
		setMessageHandler(MessageType::SERVER_INFO, handleMessageType<MessageType::SERVER_INFO>);
		setMessageHandler(MessageType::CONNECT_REQUEST, handleMessageType<MessageType::CONNECT_REQUEST>);
		setMessageHandler(MessageType::CONNECT_CHALLENGE, handleMessageType<MessageType::CONNECT_CHALLENGE>);
		setMessageHandler(MessageType::DISCONNECT, handleMessageType<MessageType::DISCONNECT>);
		setMessageHandler(MessageType::CONNECT_CONFIRM, handleMessageType<MessageType::CONNECT_CONFIRM>);
		setMessageHandler(MessageType::CONFIG_NETWORK, handleMessageType<MessageType::CONFIG_NETWORK>);

		#if ENGINE_SERVER
		if (group.port) {
			if (group.port == socket.getAddress().port) {
				ENGINE_WARN("The server port and multicast port should not be the same value(", group.port, "). May lead to instability.");
			} else {
				discoverServerSocket = Net::UDPSocket{group.port, Net::SocketFlag::NonBlocking | Net::SocketFlag::ReuseAddress};
				if (discoverServerSocket.setOption<Net::SocketOption::MulticastJoin>(group)) {
					ENGINE_LOG("LAN server discovery is available. Joining multicast group ", group);
				} else {
					ENGINE_WARN("LAN server discovery is unavailable; Unable to join multicast group ", group);
				}
			}
		} else {
			ENGINE_LOG("LAN server discovery is disabled");
		}
		#endif
	}

	#if ENGINE_CLIENT
	void NetworkingSystem::broadcastDiscover() {
		const auto now = world.getTime();
		for (auto it = servers.begin(); it != servers.end(); ++it) {
			if (it->second.lastUpdate + std::chrono::seconds{5} < now) {
				servers.erase(it);
			}
		}
		
		auto& conn = getOrCreateConnection(group);
		if (auto msg = conn.beginMessage<MessageType::DISCOVER_SERVER>()) {
			writeMessagePadding(msg.getBufferWriter());
		}
	}
	#endif

	void NetworkingSystem::recvAndDispatchMessages(Engine::Net::UDPSocket& sock) {
		int32 sz;
		Engine::Net::IPv4Address addr;

		while ((sz = sock.recv(&packet, sizeof(packet), addr)) > -1) {
			// TODO: move back to connection
			if (packet.getProtocol() != Engine::Net::protocol) {
				ENGINE_WARN("Invalid protocol");
				continue;
			}

			auto& conn = getOrCreateConnection(addr);
			if (conn.getKeyLocal() != packet.getKey()) {
				if (conn.getState() == ConnectionState::Connected) {
					ENGINE_WARN("Invalid key for ", conn.address(), " ", packet.getKey(), " != ", conn.getKeyLocal());
					continue;
				}
			}

			// ENGINE_LOG("****** ", conn.getKeySend(), " ", conn.getKeyRecv(), " ", packet.getKey(), " ", packet.getSeqNum());
			
			if (!conn.recv(packet, sz, now)) { continue; }

			while (true) {
				auto [hdr, msg] = conn.recvNext();
				if (hdr.type == 0) { break; }
				dispatchMessage(conn, hdr, msg);
				ENGINE_DEBUG_ASSERT(msg.remaining() == 0, "Incomplete read of network message.");
			}
		}
	}

	void NetworkingSystem::update(float32 dt) {
		ENGINE_DEBUG_ASSERT(disconnectingPeriod < timeout);

		now = world.getTime();

		// Recv messages
		#if ENGINE_SERVER
			recvAndDispatchMessages(discoverServerSocket);
		#endif
		recvAndDispatchMessages(socket);

		// TODO: instead of sending all connections on every X. Send a smaller number every frame to distribute load.
		const bool shouldUpdate = now - lastUpdate >= std::chrono::milliseconds{1000 / 20}; // TODO: rate should be configurable somewhere

		//
		//
		//
		//
		//
		// TODO: this network update should only run on server side.
		//
		//
		//
		//
		//
		//

		//
		//
		//
		// TODO: Connections don't have an entity until fully connected. Need to still handle those cases.
		//
		//
		//
		//
		//
		{
			// TODO: This distribution is largely untested since we don't currently
			//       have an easy way to test with a large number of players.
			constexpr Engine::Clock::Seconds netUpdateInterval = std::chrono::milliseconds{1000 / 20};
			const auto fullNetPerUpdate = Engine::Noise::floorTo<int32>(netUpdateInterval.count() / world.getDeltaTimeSmooth());

			// TODO: should probably also clamp after just in case
			ENGINE_DEBUG_ASSERT(fullNetPerUpdate >= 1.0f, "To few network updates. This is a bug.");
			ENGINE_DEBUG_ASSERT(fullNetPerUpdate <= 10.0f, "Exceptionally high number of network updates. This is likely a bug.");
			
			// Evenly distribute the remainder between runs.
			// For example, if we were doing three updates per run we would have:
			//   #plys=0:    0/3 = 0r0 = runs {0, 0, 0}
			//   #plys=1:    1/3 = 0r1 = runs {1, 0, 0}
			//   #plys=2:    2/3 = 0r2 = runs {1, 1, 0}
			//   #plys=3:    3/3 = 1r0 = runs {1, 1, 1}
			//   ...
			//   #plys=9:    9/3 = 3r0 = runs {3, 3, 3}
			//   #plys=10:  10/3 = 3r1 = runs {4, 3, 3}
			//   #plys=11:  11/3 = 3r2 = runs {4, 4, 3}
			//   #plys=12:  12/3 = 4r0 = runs {4, 4, 4}
			// 
			// Using simple division would give uneven runs since you need to
			// include the remaining players in the final loop. For example in the
			// eleven player case we would have:
			//   #plys=11:  11/3 = 3r2 = runs {3, 3, 5}
			//    
			const auto plys = world.getFilterAll<true, NetworkComponent>();
			const auto plyCountPerUpdate = Engine::Math::divFloor<int32>(plys.size(), fullNetPerUpdate);
			const auto step = static_cast<int32>(world.getUpdate() % fullNetPerUpdate);
			const auto plyCount = plyCountPerUpdate.q + (plyCountPerUpdate.r > step && plyCountPerUpdate.r > 0);
			const auto start = step * plyCountPerUpdate.q + std::min(plyCountPerUpdate.r, step);
			ENGINE_LOG2("{} + {} | {}={} / {}", start, plyCount, world.getUpdate(), step, fullNetPerUpdate);

			// This check technically isn't needed since to_address doesn't form
			// a reference. It currently exists solely so we can leave iterator
			// checking on in SparseSet::IteratorBase::operator->(). See notes
			// there for more details.
			if (plyCount > 0) {
				const NetPlySet plysThisUpdate{std::to_address(plys.begin() + start), plyCount};
				
				for (const auto& [ply, _] : plysThisUpdate) {
					ENGINE_LOG2("  Player: {}", ply);
				}
				
				Engine::Meta::ForEachIn<SystemsSet>::call([&]<class S>() ENGINE_INLINE {
					world.getSystem<S>().network(plysThisUpdate);
				});
			}
		}

		for (auto cur =  addrToConn.begin(), end = addrToConn.end(); cur != end;) {
			auto& [addr, conn] = *cur;

			// Handle any disconnects
			if (now - conn->recvTime() >= timeout) { // Timeout, havent received a message recently.
				cur = disconnect(cur);
				continue;
			} else if (conn->getState() == ConnectionState::Disconnecting) { // Graceful disconnect
				if (auto msg = conn->beginMessage<MessageType::DISCONNECT>()) {}

				if (now - conn->disconnectAt >= disconnectingPeriod) {
					cur = disconnect(cur);
					continue;
				}
			}

			// Send any messages
			if (shouldUpdate) {
				conn->send(socket);
			}
			++cur;
		}
		
		#ifdef ENGINE_UDP_NETWORK_SIM
			socket.realSimSend();
		#endif
	}

	int32 NetworkingSystem::playerCount() const {
		const auto& filter = world.getFilter<PlayerFilter>();
		return std::distance(filter.begin(), filter.end());
	}

	void NetworkingSystem::cleanECS(ConnectionInfo& conn) {
		ENGINE_INFO("ECS Clean: ", conn.ent);

		#if ENGINE_CLIENT
			// TODO: This state check isnt quite right because we have a many to one relationship. We almost want each connection to have its own mapping.
			// TODO cont: consider connected to one server > connected to second server. The first timeout would clear any entities from the second connection.
			if (conn.getState() != ConnectionState::Disconnected) {
				// TODO (uAiwkWDY): really would like a better way to handle this kind of stuff. event/signal system maybe.
				auto& entNetSys = world.getSystem<EntityNetworkingSystem>();
				bool _debug_found = !conn.ent;
				for (auto [remote, local] : entNetSys.getEntityMapping()) {
					_debug_found = _debug_found || (conn.ent && local == conn.ent);
					world.deferedDestroyEntity(local);
				}
				ENGINE_DEBUG_ASSERT(_debug_found, "Local entity not cleaned up.");
				entNetSys.clearEntityMapping();
			}
		#endif

		if (conn.ent) {
			world.removeComponent<NetworkComponent>(conn.ent);

			if constexpr (ENGINE_SERVER) { // Client should be handled above.
				world.deferedDestroyEntity(conn.ent);
			}

			conn.ent = {};
		}
	}

	auto NetworkingSystem::disconnect(ConnIt connIt) -> ConnIt {
		ENGINE_LOG("Disconnect ", connIt->second->address());
		cleanECS(*connIt->second);
		return addrToConn.erase(connIt);
	}

	void NetworkingSystem::disconnect(ConnectionInfo& conn) {
		auto found = addrToConn.find(conn.address());
		if (found == addrToConn.end()) {
			ENGINE_WARN("Missing connection mapping for connection at ", conn.address(), ". This is probably a bug.");
		} else {
			disconnect(found);
		}
	}
	
	void NetworkingSystem::requestDisconnect(const Engine::Net::IPv4Address& addr) {
		ENGINE_INFO("Requesting disconnect from ", addr);

		auto found = addrToConn.find(addr);
		if (found == addrToConn.end()) {
			ENGINE_WARN("Attempting to disconnect from a address without a connection. This is probably a bug. ", addr);
			return;
		}

		const auto conn = found->second.get();

		if (conn->getState() == ConnectionState::Disconnecting) {
			ENGINE_INFO("Duplicate disconnect request. Ignoring. ", addr);
			return;
		}

		cleanECS(*conn);
		conn->setKeyLocal(0);
		conn->setState(ConnectionState::Disconnecting);
		conn->disconnectAt = now + disconnectingPeriod;
	}

	#if ENGINE_CLIENT
	void NetworkingSystem::connectTo(const Engine::Net::IPv4Address& addr) {
		for (const auto& [add, con] : addrToConn) {
			if (con->getState() != ConnectionState::Disconnected) {
				ENGINE_WARN("Already connected to server (", add, "). Aborting.");
				return;
			}
		}

		auto& conn = getOrCreateConnection(addr);

		if (conn.getState() != ConnectionState::Disconnected) {
			ENGINE_WARN("Attempting to connect to same server while already connected. Aborting.");
			return;
		}
		
		conn.setKeyRemote(0);
		conn.setState(ConnectionState::Connecting);

		if (!conn.getKeyLocal()) {
			conn.setKeyLocal(genKey());
		}
		ENGINE_LOG("TRY CONNECT TO: ", addr, " lkey: ", conn.getKeyLocal(), " rkey: ",  conn.getKeyRemote(), " Tick: ", world.getTick());

		if (auto msg = conn.beginMessage<MessageType::CONNECT_REQUEST>()) {
			msg.write(conn.getKeyLocal());
			writeMessagePadding(msg.getBufferWriter());
		}
	}
	#endif

	void NetworkingSystem::addPlayer(ConnectionInfo& conn) {
		// TODO: i feel like this should be handled elsewhere. Where?
		ENGINE_DEBUG_ASSERT(conn.ent == Engine::ECS::INVALID_ENTITY, "Attempting to add duplicate player.");
		conn.ent = world.createEntity();
		const auto ply = conn.ent;
		world.addComponent<NetworkComponent>(ply, conn);

		ENGINE_INFO("Add player: ", ply, " ", world.hasComponent<PlayerFlag>(ply), " Tick: ", world.getTick());
		auto& physSys = world.getSystem<PhysicsSystem>();

		if constexpr (ENGINE_SERVER) {
			world.addComponent<NetworkedFlag>(ply);
			world.addComponent<ECSNetworkingComponent>(ply);
			world.addComponent<MapAreaComponent>(ply);
		} else {
			world.addComponent<CameraTargetFlag>(ply);
		}

		world.addComponent<PhysicsInterpComponent>(ply);

		world.addComponent<PlayerFlag>(ply);
		auto& spriteComp = world.addComponent<SpriteComponent>(ply);
		spriteComp.path = "assets/player.png";
		spriteComp.texture = engine.getTextureLoader().get2D(spriteComp.path);

		{
			constexpr ZoneId zoneId = 0;
			// TODO: query map system and find good spawn location
			const b2Vec2 pos = {0, 2};
			world.getSystem<ZoneManagementSystem>().addPlayer(ply, zoneId);
			
			auto& physComp = world.addComponent<PhysicsBodyComponent>(
				ply,
				physSys.createPhysicsCircle(ply, pos, zoneId, PhysicsCategory::Player)
			);

			ENGINE_WARN2("\nAdding physics comp: {} {}\n", ply, physComp.zone.id);
		}

		world.addComponent<ActionComponent>(ply, world.getTick());
	}

	ConnectionInfo& NetworkingSystem::getOrCreateConnection(const Engine::Net::IPv4Address& addr) {
		auto found = addrToConn.find(addr);
		if (found == addrToConn.end()) {
			auto [it, _] = addrToConn.emplace(addr, std::make_unique<ConnectionInfo>(addr, now));
			found = it;
			found->second->setState(ConnectionState::Disconnected);
			ENGINE_INFO2("Create connection: {} - {:0X}", addr, (intptr_t)found->second.get());
		}
		return *found->second;
	}

	void NetworkingSystem::dispatchMessage(ConnectionInfo& from, const Engine::Net::MessageHeader hdr, Engine::Net::BufferReader& msg) {
		const auto& meta = getMessageMetaInfo(hdr.type);
		constexpr auto dir = (ENGINE_SERVER ? Engine::Net::MessageDirection::ClientToServer : Engine::Net::MessageDirection::ServerToClient);
		if (!(meta.dir & dir)) {
			ENGINE_WARN("Network message ", meta.name, "(", +hdr.type, ")"," received by wrong side. Aborting.");
			ENGINE_DEBUG_ASSERT(false);
			msg.discard();
			return;
		}

		if (!(meta.recvState & from.getState())) {
			if (hdr.type != MessageType::DISCONNECT) {
				ENGINE_WARN2("Messages received in wrong state ({} - {:0X}). Aborting. {}({}) {} != {}", from.address(), (intptr_t)&from, meta.name, +hdr.type, +meta.recvState, +from.getState());
			}
			msg.discard();
			return;
		}

		if (hdr.type <= 0 || hdr.type >= MessageType::_count) {
			ENGINE_WARN("Attempting to dispatch invalid message type ", meta.name, " (", +hdr.type, ")");
		} else {
			auto func = msgHandlers[hdr.type];
			ENGINE_DEBUG_ASSERT(func, "No message handler set for type ", meta.name, " (", +hdr.type, ")");
			if (func) { func(engine, from, hdr, msg); }
		}

		if (auto rem = msg.remaining(); rem > 0) {
			ENGINE_WARN("Incomplete read of network message ", meta.name, " (", rem, " bytes remaining). Ignoring.");
		} else if (rem < 0) {
			ENGINE_WARN("Read past end of network message type ", meta.name," (", rem, " bytes remaining).");
		}
	}
}
