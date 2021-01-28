// STD
#include <set>
#include <concepts>
#include <iomanip>
#include <random>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ECS/Entity.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/comps/ConnectionComponent.hpp>

namespace {
	using PlayerFilter = Engine::ECS::EntityFilterList<
		Game::PlayerFlag,
		Game::ConnectionComponent
	>;

	template<class T>
	concept IsNetworkedComponent = requires (T t) {
		Engine::Net::Replication{t.netRepl()};
	};

	// TODO: would probably be easier to just have a base class instead of all these type traits
	template<class T, class = void>
	struct GetComponentReplication {
		constexpr static auto value = Engine::Net::Replication::NONE;
	};

	// TODO: move into meta
	template<class ComponentsSet>
	struct ForEachIn {
		template<class Func>
		static void call(Func&& func) {};
	};

	template<template<class...> class ComponentsSet, class... Components>
	struct ForEachIn<ComponentsSet<Components...>> {
		template<class Func>
		static void call(Func&& func) {
			(func.operator()<Components>(), ...);
		}
	};

	// TODO: figure out a good pattern
	constexpr uint8 MESSAGE_PADDING_DATA[Engine::Net::MAX_MESSAGE_SIZE - sizeof(uint16)] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22,
	};
}
#if DEBUG
namespace {
	struct HandleMessageDef_DebugBreak_Struct {
		const Engine::Net::MessageHeader* hdr;
		Game::Connection& from;
		const char* msgType = nullptr;

		~HandleMessageDef_DebugBreak_Struct() {
			using byte = Engine::byte;
			const byte* stop = reinterpret_cast<const byte*>(hdr) + sizeof(*hdr) + hdr->size;
			const byte* curr = static_cast<const byte*>(from.read(0));
			const auto rem = stop - curr;
			//if (rem != 0) { __debugbreak(); }
		}
	};
}
#define HandleMessageDef_DebugBreak(MsgType) HandleMessageDef_DebugBreak_Struct _ENGINE_temp_object_for_HandleMessageDef_DebugBreak_Struct{&head, from, MsgType}
#else
#define HandleMessageDef_DebugBreak(...)
#endif

#define HandleMessageDef(MsgType)\
	template<> void NetworkingSystem::handleMessageType<MsgType>(ConnInfo& info, Connection& from, const Engine::Net::MessageHeader& head) {\
	if constexpr (!(MessageType_Traits<MsgType>::side & ENGINE_SIDE)) { ENGINE_WARN("HandleMessageDef Abort A"); return; }\
	if (!(MessageType_Traits<MsgType>::state & info.state)) { from.read(from.recvMsgSize()); ENGINE_WARN("HandleMessageDef Abort B: ", (int)head.type); return; }\
	HandleMessageDef_DebugBreak(#MsgType);

namespace Game {
	HandleMessageDef(MessageType::UNKNOWN)
	}

	HandleMessageDef(MessageType::DISCOVER_SERVER)
		// TODO: rate limit per ip (longer if invalid packet)
		constexpr auto size = sizeof(MESSAGE_PADDING_DATA);
		if (from.recvMsgSize() == size && !memcmp(from.read(size), MESSAGE_PADDING_DATA, size)) {
			if (from.msgBegin<MessageType::SERVER_INFO>()) {
				constexpr char name[] = "This is the name of the server";
				from.write<int>(sizeof(name) - 1);
				from.write(name, sizeof(name) - 1);
				from.msgEnd<MessageType::SERVER_INFO>();
			}
		}
	}
	
	HandleMessageDef(MessageType::SERVER_INFO)
		#if ENGINE_CLIENT
			auto& servInfo = servers[from.address()];
			const auto* len = from.read<int>();
			const char* name = static_cast<const char*>(from.read(*len));
			servInfo.name.assign(name, *len);
			servInfo.lastUpdate = Engine::Clock::now();
		#endif
	}

	HandleMessageDef(MessageType::CONNECT_REQUEST)
		constexpr auto size = sizeof(MESSAGE_PADDING_DATA);
		if (from.recvMsgSize() != size + sizeof(uint16) || memcmp(from.read(size), MESSAGE_PADDING_DATA, size)) {
			// TODO: rate limit connections with invalid messages
			ENGINE_WARN("Got invalid connection request from ", from.address());
			return;
		}

		info.state = std::max(info.state, ConnState::Connecting);
		const auto keySend = *from.read<uint16>();
		if (!from.getKeySend()) {
			from.setKeySend(keySend);
		}
		if (!from.getKeyRecv()) {
			from.setKeyRecv(genKey());
		}
		if (from.msgBegin<MessageType::CONNECT_CHALLENGE>()) {
			from.write(from.getKeyRecv());
			from.msgEnd<MessageType::CONNECT_CHALLENGE>();
			ENGINE_LOG("MessageType::CONNECT_REQUEST ", from.getKeyRecv(), " ", (int)info.state);
		}
	}
	
	HandleMessageDef(MessageType::CONNECT_CHALLENGE)
		const auto& keySend = *from.read<uint16>();
		if (!from.getKeySend()) {
			from.setKeySend(keySend);
		}
		if (from.msgBegin<MessageType::CONNECT_CONFIRM>()) {
			from.write(from.getKeySend());
			from.msgEnd<MessageType::CONNECT_CONFIRM>();
			info.state = ConnState::Connected;
		}
		ENGINE_LOG("MessageType::CONNECT_CHALLENGE from ", from.address(), " ", keySend);
	}

	HandleMessageDef(MessageType::CONNECT_CONFIRM)
		const auto key = *from.read<uint16>();
		if (key != from.getKeyRecv()) {
			ENGINE_WARN("CONNECT_CONFIRM: Invalid recv key ", key, " != ", from.getKeyRecv());
			return;
		}

		info.state = ConnState::Connected;
		addPlayer(info.ent);

		ENGINE_LOG("SERVER MessageType::CONNECT_CONFIRM", " Tick: ", world.getTick());
		// TODO: change message type of this (for client). This isnt a confirmation this is initial sync or similar.
		if (from.msgBegin<MessageType::ECS_INIT>()) {
			from.write(info.ent);
			from.write(world.getTick());
			from.msgEnd<MessageType::ECS_INIT>();
		}
	}

	HandleMessageDef(MessageType::ECS_INIT)
		auto* remote = from.read<Engine::ECS::Entity>();
		auto* tick = from.read<Engine::ECS::Tick>();

		if (!remote) {
			ENGINE_WARN("Server didn't send remote entity. Unable to sync.");
			return;
		}

		if (!tick) {
			ENGINE_WARN("Unable to sync ticks.");
			return;
		}

		addPlayer(info.ent);
		entToLocal[*remote] = info.ent;
		ENGINE_LOG("ECS_INIT - Remote: ", *remote, " Local: ", info.ent, " Tick: ", world.getTick(), " - ", *tick);

		// TODO: use ping, loss, etc to pick good offset value. We dont actually have good quality values for those stats yet at this point.
		world.setNextTick(*tick + 16);
	}

	HandleMessageDef(MessageType::DISCONNECT)
		ENGINE_LOG("MessageType::DISCONNECT ", from.address(), " ", info.ent);
		if (info.state != Engine::Net::ConnState::Connected) { return; }
		if (info.disconnectAt != Engine::Clock::TimePoint{}) { return; }
		info.disconnectAt = Engine::Clock::now() + disconnectTime;
		info.state = Engine::Net::ConnState::Disconnected;
	}

	HandleMessageDef(MessageType::PING)
		static uint8 last = 0;
		// TODO: nullptr check
		const auto data = *from.read<uint8>();
		const bool pong = data & 0x80;
		const int32 val = data & 0x7F;
		last = val;

		if (pong) {
			ENGINE_LOG("recv pong @ ",
				std::fixed, std::setprecision(2),
				Engine::Clock::now().time_since_epoch().count() / 1E9,
				" from ", from.address(),
				" ", val
			);
		} else {
			ENGINE_LOG("recv ping @ ",
				std::fixed, std::setprecision(2),
				Engine::Clock::now().time_since_epoch().count() / 1E9,
				" from ", from.address(),
				" ", val
			);
			if (from.msgBegin<MessageType::PING>()) {
				from.write(static_cast<uint8>(val | 0x80));
				from.msgEnd<MessageType::PING>();
			}
		}
	}
	
	HandleMessageDef(MessageType::ECS_ENT_CREATE)
		auto* remote = from.read<Engine::ECS::Entity>();
		if (!remote) { return; }

		auto& local = entToLocal[*remote];
		if (local == Engine::ECS::INVALID_ENTITY) {
			local = world.createEntity();
		}

		world.addComponent<NetworkedFlag>(local);
		ENGINE_LOG("Networked: ", local, world.hasComponent<NetworkedFlag>(local));

		ENGINE_LOG("ECS_ENT_CREATE - Remote: ", *remote, " Local: ", local, " Tick: ", world.getTick());

		// TODO: components init
	}

	HandleMessageDef(MessageType::ECS_ENT_DESTROY) 
		auto* remote = from.read<Engine::ECS::Entity>();
		if (!remote) { return; }
		auto found = entToLocal.find(*remote);
		if (found != entToLocal.end()) {
			world.deferedDestroyEntity(found->second);
			entToLocal.erase(found);
		}
	}

	HandleMessageDef(MessageType::ECS_COMP_ADD)
		const auto* remote = from.read<Engine::ECS::Entity>();
		const auto* cid = from.read<Engine::ECS::ComponentId>();
		if (!remote || !cid) { return; }

		auto found = entToLocal.find(*remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		world.callWithComponent(*cid, [&]<class C>(){
			if constexpr (IsNetworkedComponent<C>) {
				if (!world.hasComponent<C>(local)) {
					auto& comp = world.addComponent<C>(local);
					comp.netFromInit(engine, world, local, from);
				}
			} else {
				ENGINE_WARN("Attemping to network non-network component");
			}
		});
	}

	HandleMessageDef(MessageType::ECS_COMP_ALWAYS)
		const auto* remote = from.read<Engine::ECS::Entity>();
		const auto* cid = from.read<Engine::ECS::ComponentId>();
		if (!remote || !cid) { return; }
		
		auto found = entToLocal.find(*remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		if (!world.isAlive(local)) {
			ENGINE_WARN("Attempting to update dead entitiy ", local);
			return;
		}

		if (!world.hasComponent(local, *cid)) {
			ENGINE_WARN(local, " does not have component ", *cid);
			return;
		}

		world.callWithComponent(*cid, [&]<class C>(){
			if constexpr (IsNetworkedComponent<C>) {
				// TODO: this is a somewhat strange way to handle this
				if constexpr (Engine::ECS::IsSnapshotRelevant<C>::value) {
					const auto* tick = from.read<Engine::ECS::Tick>();
					if (!tick) {
						ENGINE_WARN("No tick specified for snapshot component in ECS_COMP_ALWAYS");
						return;
					}

					auto& comp = world.getComponent<C>(local, *tick);
					comp.netFrom(from);
				} else {
					world.getComponent<C>(local).netFrom(from);
				}
			} else {
				ENGINE_WARN("Attemping to network non-network component");
			}
		});
	}

	HandleMessageDef(MessageType::ECS_FLAG)
		const auto remote = from.read<Engine::ECS::Entity>();
		const auto flags = from.read<Engine::ECS::ComponentBitset>();
		if (!remote || !flags) { return; }

		auto found = entToLocal.find(*remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		ForEachIn<ComponentsSet>::call([&]<class C>() {
			constexpr auto cid = world.getComponentId<C>();
			if constexpr (!Engine::ECS::IsFlagComponent<C>::value) { return; }
			if (!flags->test(cid)) { return; }

			if (world.hasComponent<C>(local)) {
				world.removeComponent<C>(local);
			} else {
				world.addComponent<C>(local);
			}
		});
	}
	
	HandleMessageDef(MessageType::PLAYER_DATA)
		const auto* tick = from.read<Engine::ECS::Tick>();
		const auto* trans = from.read<b2Transform>();
		const auto* vel = from.read<b2Vec2>();

		if (!tick || !trans || !vel) {
			ENGINE_WARN("Invalid PLAYER_DATA network message");
			return;
		}

		if (!world.hasComponent<PhysicsProxyComponent>(info.ent)) {
			ENGINE_WARN("PLAYER_DATA message received for entity that has no PhysicsProxyComponent");
			return;
		}

		auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(info.ent, *tick);
		const auto diff = physProxyComp.trans.p - trans->p;
		const float32 eps = 0.0001f; // TODO: figure out good eps value. Probably half the size of a pixel or similar.
		//if (diff.LengthSquared() > 0.0001f) { // TODO: also check q
		// TODO: why does this ever happen with only one player connected?
		if (diff.LengthSquared() >  eps * eps) { // TODO: also check q
			ENGINE_INFO(std::setprecision(std::numeric_limits<decltype(physProxyComp.trans.p.x)>::max_digits10),
				"Oh boy a mishap has occured on tick ", *tick,
				" (<", physProxyComp.trans.p.x, ", ", physProxyComp.trans.p.y, "> - <",
				trans->p.x, ", ", trans->p.y, "> = <",
				diff.x, ", ", diff.y,
				">)"
			);
			physProxyComp.trans = *trans;
			physProxyComp.vel = *vel;
			physProxyComp.rollbackOverride = true;

			world.scheduleRollback(*tick);
		}

		// TODO: vel
	}

	HandleMessageDef(MessageType::ACTION)
		world.getSystem<ActionSystem>().recvActions(from, head, info.ent);
	}

	HandleMessageDef(MessageType::TEST)
	}

	HandleMessageDef(MessageType::MAP_CHUNK)
		world.getSystem<MapSystem>().chunkFromNet(from, head);
	}

	// TODO: unsued?
	HandleMessageDef(MessageType::SPELL)
		auto& spellSys = world.getSystem<CharacterSpellSystem>();
		const auto* pos = from.read<b2Vec2>();
		const auto* dir = from.read<b2Vec2>();
		if (!pos || !dir) { return; }
		spellSys.queueMissile(*pos, *dir);
	}
}
#undef HandleMessageDef

namespace Game {
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, socket{ENGINE_SERVER ? Engine::getGlobalConfig().port : 0}
		, group{Engine::getGlobalConfig().group}
		, rng{pcg_extras::seed_seq_from<std::random_device>{}} {

		ENGINE_LOG("Listening on port ", socket.getAddress().port);

		if constexpr (ENGINE_SERVER) {
			if (socket.setOption<Engine::Net::SocketOption::MULTICAST_JOIN>(group)) {
				ENGINE_LOG("LAN server discovery is available. Joining multicast group ", group);
			} else {
				ENGINE_WARN("LAN server discovery is unavailable; Unable to join multicast group ", group);
			}
		}
	}

	auto NetworkingSystem::getOrCreateConnection(const Engine::Net::IPv4Address& addr) -> AddConnRes {
		ConnInfo* info;
		Connection* conn;
		const auto found = connections.find(addr);
		if (found == connections.end()) {
			const auto& [i, c] = addConnection2(addr);
			info = &i;
			conn = &c;
		} else {
			info = &found->second;
			conn = world.getComponent<ConnectionComponent>(info->ent).conn.get();
		}
		return {*info, *conn};
	}

	#if ENGINE_CLIENT
	void NetworkingSystem::broadcastDiscover() {
		const auto now = Engine::Clock::now();
		for (auto it = servers.begin(); it != servers.end(); ++it) {
			if (it->second.lastUpdate + std::chrono::seconds{5} < now) {
				servers.erase(it);
			}
		}

		const auto& [info, conn] = getOrCreateConnection(group);
		if (conn.msgBegin<MessageType::DISCOVER_SERVER>()) {
			conn.write(MESSAGE_PADDING_DATA);
			conn.msgEnd<MessageType::DISCOVER_SERVER>();
		}
	}
	#endif

	void NetworkingSystem::run(float32 dt) {
		now = Engine::Clock::now();

		// Recv messages
		int32 sz;
		while ((sz = socket.recv(&packet, sizeof(packet), address)) > -1) {
			const auto& [info, conn] = getOrCreateConnection(address);
			// TODO: move back to connection
			if (packet.getProtocol() != Engine::Net::protocol) {
				ENGINE_WARN("Invalid protocol"); // TODO: rm - could be used for lag/dos?
				continue;
			}

			if (conn.getKeyRecv() != packet.getKey()) {
				if (info.state == Engine::Net::ConnState::Connected) {
					ENGINE_WARN("Invalid key for ", conn.address(), " ", packet.getKey(), " != ", conn.getKeyRecv());
					continue;
				}
			}

			// ENGINE_LOG("****** ", conn.getKeySend(), " ", conn.getKeyRecv(), " ", packet.getKey(), " ", packet.getSeqNum());

			if (!conn.recv(packet, sz, now)) { continue; }

			const Engine::Net::MessageHeader* hdr; 
			while (hdr = conn.recvNext()) {
				dispatchMessage(info, conn, hdr);
			}
		}

		// TODO: instead of sending all connections on every X. Send a smaller number every frame to distribute load.

		// Send messages
		// TODO: rate should be configurable somewhere
		const bool shouldUpdate = now - lastUpdate >= std::chrono::milliseconds{1000 / 20};

		if (shouldUpdate) {
			lastUpdate = now;
			if constexpr (ENGINE_SERVER) { runServer(); }
			if constexpr (ENGINE_CLIENT) { runClient(); }

			for (auto& [addr, info] : connections) {
				auto& conn = *world.getComponent<ConnectionComponent>(info.ent).conn;
				const auto diff = now - conn.recvTime();
				if (diff > timeout) {
					ENGINE_LOG("Connection for ", info.ent ," (", addr, ") timed out.");
					info.disconnectAt = Engine::Clock::now() - disconnectTime;
					info.state = Engine::Net::ConnState::Disconnected;
				}
			}
		}

		// Send Ack messages & unacked
		for (auto it = connections.begin(); it != connections.end();) {
			auto& [addr, info] = *it;
			const auto ply = info.ent;
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;

			// TODO: why does moving this to right before send (under writeUnacked?) cause a crash?c
			//if (info.state == Engine::Net::ConnState::Connected) {
			//	conn.msgBegin<MessageType::TEST>();
			//	conn.msgEnd<MessageType::TEST>();
			//}
			
			if (info.disconnectAt != Engine::Clock::TimePoint{}) {
				// TODO: remove all comps but connection

				if (info.state == Engine::Net::ConnState::Connected) {
					ENGINE_LOG("Send MessageType::DISCONNECT to ", addr);
					conn.msgBegin<MessageType::DISCONNECT>();
					conn.msgEnd<MessageType::DISCONNECT>();
				}

				if (info.disconnectAt < now) {
					conn.send(socket);
					conn.setKeySend(0);
					conn.setKeyRecv(0);
					info.state = Engine::Net::ConnState::Disconnected;

					ENGINE_LOG("Disconnecting ", info.ent, " ", addr);
					// TODO: world.removeComponent<ConnectionComponent>(info.ent);
					world.deferedDestroyEntity(info.ent);
					it = connections.erase(it);
					continue;
				}
			} else if (shouldUpdate) {
				conn.writeUnacked(socket);
			}

			conn.send(socket);
			++it;
		}

		#ifdef ENGINE_UDP_NETWORK_SIM
			socket.simPacketSend();
		#endif
	}

	void NetworkingSystem::runServer() {
		updateNeighbors();
		if (world.getAllComponentBitsets().size() > lastCompsBitsets.size()) {
			lastCompsBitsets.resize(world.getAllComponentBitsets().size());
		}

		for (auto& ply : world.getFilter<PlayerFilter>()) {
			auto& neighComp = world.getComponent<NeighborsComponent>(ply);
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;

			{ // TODO: player data should be sent every tick along with actions/inputs.
			// TODO: cont.  Should it? every few frames is probably fine for keeping it in sync. Although when it does desync it will be a larger rollback.
				auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
				if (conn.msgBegin<MessageType::PLAYER_DATA>()) {
					conn.write(world.getTick() + 1); // since this is in `run` and not before `tick` we are sending one tick off. +1 is temp fix
					conn.write(physComp.getTransform());
					conn.write(physComp.getVelocity());
					conn.msgEnd<MessageType::PLAYER_DATA>();
				}
			}

			// TODO: handle entities without phys comp?
			// TODO: figure out which entities have been updated
			// TODO: prioritize entities
			// TODO: figure out which comps on those entities have been updated

			for (const auto& pair : neighComp.addedNeighbors) {
				ENGINE_DEBUG_ASSERT(pair.first != ply, "A player is not their own neighbor");
				const auto& ent = pair.first;

				if (conn.msgBegin<MessageType::ECS_ENT_CREATE>()) { // TODO: General_RO;
					conn.write(ent);
					conn.msgEnd<MessageType::ECS_ENT_CREATE>();
				}

				ForEachIn<ComponentsSet>::call([&]<class C>() {
					if constexpr (IsNetworkedComponent<C>) {
						ENGINE_LOG("IsNetworkedComponent ", world.getComponentId<C>());
						if (!world.hasComponent<C>(ent)) { return; }

						auto& comp = world.getComponent<C>(ent);
						if (comp.netRepl() == Engine::Net::Replication::NONE) { return; }

						if (conn.msgBegin<MessageType::ECS_COMP_ADD>()) {//, General_RO);
							conn.write(ent);
							conn.write(world.getComponentId<C>());
							comp.netToInit(engine, world, ent, conn);
							conn.msgEnd<MessageType::ECS_COMP_ADD>();
						}
					}
				});
			}

			for (const auto& pair : neighComp.removedNeighbors) {
				ENGINE_DEBUG_ASSERT(pair.first != ply, "A player is not their own neighbor");
				if (conn.msgBegin<MessageType::ECS_ENT_DESTROY>()) { // TODO: General_RO;
					conn.write(pair.first);
					conn.msgEnd<MessageType::ECS_ENT_DESTROY>();
				}
			}

			for (const auto& pair : neighComp.currentNeighbors) {
				ENGINE_DEBUG_ASSERT(pair.first != ply, "A player is not their own neighbor");
				const auto ent = pair.first;
				Engine::ECS::ComponentBitset flagComps;

				ForEachIn<ComponentsSet>::call([&]<class C>() {
					// TODO: Note: this only updates components not flags. Still need to network flags.
					constexpr auto cid = world.getComponentId<C>();
					if constexpr (IsNetworkedComponent<C>) {
						if (!world.hasComponent<C>(ent)) { return; }
						const auto& comp = world.getComponent<C>(ent);
						const auto repl = comp.netRepl();
						const int32 diff = lastCompsBitsets[ent.id].test(cid) - world.getComponentsBitset(ent).test(cid);

						// TODO: repl then diff. not diff then repl

						if (diff < 0) { // Component Added
							// TODO: comp added
							// TODO: currently this is duplicate with addedNeighbors init
							//conn.writer.next(MessageType::ECS_COMP_ADD, Engine::Net::Channel::ORDERED);
							//conn.writer.write(cid);
							//comp.netToInit(world, ent, conn.writer);
						} else if (diff > 0) { // Component Removed
							// TODO: comp removed
						} else { // Component Updated
							if (repl == Engine::Net::Replication::ALWAYS) {
								if (!conn.msgBegin<MessageType::ECS_COMP_ALWAYS>()) { return; }
								conn.write(ent);
								conn.write(cid);
								if (Engine::ECS::IsSnapshotRelevant<C>::value) {
									conn.write(world.getTick());
								}
								comp.netTo(conn);
								conn.msgEnd<MessageType::ECS_COMP_ALWAYS>();
							} else if (repl == Engine::Net::Replication::UPDATE) {
								// TODO: impl
							}
						}
					} else if constexpr (Engine::ECS::IsFlagComponent<C>::value) {
						const int32 diff = lastCompsBitsets[ent.id].test(cid) - world.getComponentsBitset(ent).test(cid);
						if (diff) {
							flagComps.set(cid);
						}
					}
				});

				if (flagComps) {
					if (conn.msgBegin<MessageType::ECS_FLAG>()) { // TODO: General_RO
						conn.write(ent);
						conn.write(flagComps);
						conn.msgEnd<MessageType::ECS_FLAG>();
					}
				}
			}
		}

		lastCompsBitsets = world.getAllComponentBitsets();
	}

	
	void NetworkingSystem::runClient() {
		for (auto& [addr, info] : connections) {
			if (info.state == ConnState::Connecting) {
				connectTo(addr);
				break;
			} else if (info.state == ConnState::Connected) {
				static uint8 ping = 0;
				static auto next = now;
				if (next > now) { return; }
				next = now + std::chrono::milliseconds{5000};
		
				for (auto& ply : world.getFilter<PlayerFilter>()) {
					auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;

					if (auto msg = conn.beginMessage<MessageType::PING>()) {
						msg.write(static_cast<uint8>(++ping & 0x7F));
					}
				}
			}
		}
	}

	int32 NetworkingSystem::connectionsCount() const {
		return static_cast<int32>(connections.size());
	}

	int32 NetworkingSystem::playerCount() const {
		return static_cast<int32>(world.getFilter<PlayerFilter>().size());
	}

	void NetworkingSystem::connectTo(const Engine::Net::IPv4Address& addr) {
		const auto& [info, conn] = getOrCreateConnection(addr);
		info.state = ConnState::Connecting;

		if (!conn.getKeyRecv()) {
			conn.setKeyRecv(genKey());
		}
		ENGINE_LOG("TRY CONNECT TO: ", addr, " rkey: ", conn.getKeyRecv(), " skey: ",  conn.getKeySend(), " Tick: ", world.getTick());

		if (conn.msgBegin<MessageType::CONNECT_REQUEST>()) {
			conn.write(MESSAGE_PADDING_DATA);
			conn.write(conn.getKeyRecv());
			conn.msgEnd<MessageType::CONNECT_REQUEST>();
		}
	}

	auto NetworkingSystem::addConnection2(const Engine::Net::IPv4Address& addr) -> AddConnRes {
		auto ent = world.createEntity();
		ENGINE_INFO("Add connection: ", ent, " ", addr, " ", world.hasComponent<PlayerFlag>(ent), " ");
		auto [it, suc] = connections.emplace(addr, ConnInfo{
			.ent = ent,
			.state = ConnState::Disconnected,
		});
		auto& connComp = world.addComponent<ConnectionComponent>(ent);
		connComp.conn = std::make_unique<Connection>(addr, now);
		return {it->second, *connComp.conn};
	}

	void NetworkingSystem::addPlayer(const Engine::ECS::Entity ent) {
		// TODO: i feel like this should be handled elsewhere. Where?

		ENGINE_INFO("Add player: ", ent, " ", world.hasComponent<PlayerFlag>(ent), " Tick: ", world.getTick());
		auto& physSys = world.getSystem<PhysicsSystem>();

		if constexpr (ENGINE_SERVER) {
			world.addComponent<NetworkedFlag>(ent);
			world.addComponent<NeighborsComponent>(ent);
			world.addComponent<MapAreaComponent>(ent);
		} else {
			world.addComponent<CameraTargetFlag>(ent);
		}
		// TODO: client only
		world.addComponent<PhysicsInterpComponent>(ent);

		world.addComponent<PlayerFlag>(ent);
		world.addComponent<SpriteComponent>(ent).texture = engine.textureManager.get("assets/player.png");

		auto& physComp = world.addComponent<PhysicsBodyComponent>(ent);
		physComp.setBody(physSys.createPhysicsCircle(ent, {}, -+PhysicsType::Player));
		physComp.type = PhysicsType::Player;

		world.addComponent<PhysicsProxyComponent>(ent);
		world.addComponent<ActionComponent>(ent);
		world.addComponent<MapEditComponent>(ent);
		world.addComponent<CharacterSpellComponent>(ent);
	}

	void NetworkingSystem::requestDisconnect(const Engine::Net::IPv4Address& addr) {
		const auto& [info, conn] = getOrCreateConnection(addr);
		info.disconnectAt = Engine::Clock::now() + disconnectTime;
		ENGINE_INFO("Request disconnect ", addr, " ", info.ent);
	}

	void NetworkingSystem::dispatchMessage(ConnInfo& info, Connection& from, const Engine::Net::MessageHeader* hdr) {
		constexpr auto msgToStr = [](const Engine::Net::MessageType& type) -> const char* {
			#define X(Name, Side, State) if (type == MessageType::Name) { return #Name; }
			#include <Game/MessageType.xpp>
			return "UNKNOWN";
		};

		switch(hdr->type) {
			#define X(Name, Side, State) case MessageType::Name: { handleMessageType<MessageType::Name>(info, from, *hdr); break; };
			#include <Game/MessageType.xpp>
			default: {
				ENGINE_WARN("Unhandled network message type ", static_cast<int32>(hdr->type));
			}
		}
		
		const byte* stop = reinterpret_cast<const byte*>(hdr) + sizeof(*hdr) + hdr->size;
		const byte* curr = static_cast<const byte*>(from.read(0));
		const auto rem = stop - curr;

		if (rem > 0) {
			ENGINE_WARN("Incomplete read of network message ", msgToStr(hdr->type), " (", rem, " bytes remaining). Ignoring.");
			from.read(rem);
		} else if (rem < 0) {
			ENGINE_WARN("Read past end of network messge type ", msgToStr(hdr->type)," (", rem, " bytes remaining).");
		}
	}

	void NetworkingSystem::updateNeighbors() {
		for (const auto ply : world.getFilter<PlayerFilter>()) {
			auto& neighComp = world.getComponent<NeighborsComponent>(ply);
			auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
			auto& added = neighComp.addedNeighbors;
			auto& current = neighComp.currentNeighbors;
			auto& removed = neighComp.removedNeighbors;
			{using std::swap; swap(lastNeighbors, current);};
			added.clear();
			current.clear();
			removed.clear();

			struct QueryCallback : b2QueryCallback {
				decltype(current)& ents;
				World& world;
				QueryCallback(World& world, decltype(ents) ents) : world{world}, ents{ents} {}
				virtual bool ReportFixture(b2Fixture* fixture) override {
					const Engine::ECS::Entity ent = Game::PhysicsSystem::toEntity(fixture->GetBody()->GetUserData());
					if (!world.hasComponent<NetworkedFlag>(ent)) { return true; }
					if (!ents.has(ent)) { ents.add(ent); }
					return true;
				}
			} queryCallback(world, current);

			const auto& pos = physComp.getPosition();
			constexpr float32 range = 5; // TODO: what range?
			physComp.getWorld()->QueryAABB(&queryCallback, b2AABB{
				{pos.x - range, pos.y - range},
				{pos.x + range, pos.y + range},
			});

			if (current.has(ply)) {
				current.remove(ply);
			}

			for (const auto lent : lastNeighbors) {
				if (!current.has(lent.first)) {
					ENGINE_LOG("removed: ", lent.first);
					removed.add(lent.first);
				}
			}

			for (const auto cent : current) {
				if (!lastNeighbors.has(cent.first)) {
					ENGINE_LOG("removed: ", cent.first);
					added.add(cent.first);
				}
			}

			// TODO: disabled vs destroyed
		}
	}
}
