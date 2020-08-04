// STD
#include <set>
#include <concepts>
#include <iomanip>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ECS/Entity.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/comps/ConnectionComponent.hpp>

namespace {
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
		static void call(Func& func) {};
	};

	template<template<class...> class ComponentsSet, class... Components>
	struct ForEachIn<ComponentsSet<Components...>> {
		template<class Func>
		static void call(Func& func) {
			(func.operator()<Components>(), ...);
		}
	};

	// TODO: figure out a good pattern
	constexpr uint8 DISCOVER_SERVER_DATA[Engine::Net::MAX_MESSAGE_SIZE] = {
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
		0x00, 0x11, 0x22, 0x33, 0x44,
	};
}


#define HandleMessageDef(MsgType) template<> void NetworkingSystem::handleMessageType<MsgType>(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt)
namespace Game {
	HandleMessageDef(MessageType::UNKNOWN) {
	}

	HandleMessageDef(MessageType::DISCOVER_SERVER) {
		constexpr auto size = sizeof(DISCOVER_SERVER_DATA);

		// TODO: rate limit per ip (longer if invalid packet)

		if (from.recvMsgSize() == size && !memcmp(from.read(size), DISCOVER_SERVER_DATA, size)) {
			from.msgBegin<MessageType::SERVER_INFO>();
			from.write("This is the name of the server");
			from.msgEnd<MessageType::SERVER_INFO>();
		}
	}
	
	HandleMessageDef(MessageType::SERVER_INFO) {
		#if ENGINE_CLIENT
			auto& info = servers[from.address()];
			info.name = from.read<char*>();
			info.lastUpdate = Engine::Clock::now();
		#endif
	}

	HandleMessageDef(MessageType::CONNECT) {
		// TODO: since messages arent reliable do connect/disconnect really make any sense?
		ENGINE_LOG("MessageType::CONNECT from ", from.address());
		addPlayer(fromEnt);

		const auto* tick = from.read<Engine::ECS::Tick>();

		from.msgBegin<MessageType::CONNECT_CONFIRM>();
		from.write(fromEnt);
		from.write(world.getTick());
		from.write(*tick);
		from.msgEnd<MessageType::CONNECT_CONFIRM>();
	}
	
	HandleMessageDef(MessageType::CONNECT_CONFIRM) {
		auto* remote = from.read<Engine::ECS::Entity>();
		auto* rtick = from.read<Engine::ECS::Tick>();
		auto* ltick = from.read<Engine::ECS::Tick>();

		if (!remote) {
			ENGINE_WARN("Server didn't send remote entity. Unable to sync.");
			return;
		}

		ENGINE_LOG("Ticks: ", *rtick, " ", *ltick);

		entToLocal[*remote] = fromEnt;
		ENGINE_LOG("Connection established. Remote: ", *remote, " Local: ", fromEnt);

		if constexpr (ENGINE_CLIENT) {
			addPlayer(fromEnt);
		}
	}

	HandleMessageDef(MessageType::DISCONNECT) {
		puts("MessageType::DISCONNECT");
		disconnect(fromEnt);
	}

	HandleMessageDef(MessageType::PING) {
		static uint8 last = 0;
		// TODO: nullptr check
		const auto data = *from.read<uint8>();
		const bool pong = data & 0x80;
		const int32 val = data & 0x7F;

		//if (val != ((last + 1) & 0x7F)) {
		//	ENGINE_WARN("\n\n**** OUT OF ORDER ****\n");
		//	//__debugbreak();
		//}
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
			from.msgBegin<MessageType::PING>();
			from.write(static_cast<uint8>(val | 0x80));
			from.msgEnd<MessageType::PING>();
		}
	}
	
	HandleMessageDef(MessageType::ECS_ENT_CREATE) {
		// TODO: this message should be client only
		auto* remote = from.read<Engine::ECS::Entity>();
		if (!remote) { return; }

		auto& local = entToLocal[*remote];
		if (local == Engine::ECS::INVALID_ENTITY) {
			local = world.createEntity();
		}

		world.setNetworked(local, true);
		ENGINE_LOG("Networked: ", local, world.isNetworked(local));

		ENGINE_LOG("ECS_ENT_CREATE - Remote: ", *remote, " Local: ", local);

		// TODO: components init
	}

	HandleMessageDef(MessageType::ECS_ENT_DESTROY) {
		// TODO: this message should be client only
		auto* remote = from.read<Engine::ECS::Entity>();
		if (!remote) { return; }
		auto found = entToLocal.find(*remote);
		if (found != entToLocal.end()) {
			world.deferedDestroyEntity(found->second);
			entToLocal.erase(found);
		}
	}

	HandleMessageDef(MessageType::ECS_COMP_ADD) {
		// TODO: this message should be client only
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

	HandleMessageDef(MessageType::ECS_COMP_ALWAYS) {
		// TODO: this message should be client only
		const auto* remote = from.read<Engine::ECS::Entity>();
		const auto* cid = from.read<Engine::ECS::ComponentId>();
		if (!remote || !cid) { return; }

		auto found = entToLocal.find(*remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;
		// TODO: What if we get an older message after a newer message?
		//ENGINE_LOG("Update: ", head.sequence);
		world.callWithComponent(*cid, [&]<class C>(){
			if constexpr (IsNetworkedComponent<C>) {
				if (world.hasComponent<C>(local)) {
					world.getComponent<C>(local).netFrom(from);
				}
			} else {
				ENGINE_WARN("Attemping to network non-network component");
			}
		});
	}

	HandleMessageDef(MessageType::ECS_FLAG) {
		// TODO: this message should be client only
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

	HandleMessageDef(MessageType::ACTION) {
		world.getSystem<ActionSystem>().recvActions(from, head, fromEnt);
		//
		//const auto* aid = from.reader.read<Engine::Input::ActionId>();
		//const auto* val = from.reader.read<Engine::Input::Value>();
		//
		//if (aid && val) {
		//	if (*aid > 1) {
		//		ENGINE_LOG("Net: ", *aid, " ", val->value, " ", world.getTick());
		//	}
		//
		//	// TODO: sanity check inputs
		//	world.getSystem<ActionSystem>().queueAction(fromEnt, *aid, *val);
		//}
	}

	HandleMessageDef(MessageType::TEST) {
		std::cout << "***** TEST: " << head.seq << "\n";
	}
}
#undef HandleMessageDef

namespace Game {
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, socket{ENGINE_SERVER ? *engine.commandLineArgs.get<uint16>("port") : 0}
		, connFilter{world.getFilterFor<ConnectionComponent>()}
		, plyFilter{world.getFilterFor<PlayerFlag>()}
		, group{*engine.commandLineArgs.get<Engine::Net::IPv4Address>("group")} {

		ENGINE_LOG("Listening on port ", socket.getAddress().port);

		if (socket.setOption<Engine::Net::SocketOption::MULTICAST_JOIN>(group)) {
			ENGINE_LOG("LAN server discovery is available. Joining multicast group ", group);
		} else {
			ENGINE_WARN("LAN server discovery is unavailable; Unable to join multicast group ", group);
		}
	}

	void NetworkingSystem::setup() {
	}

	
	auto NetworkingSystem::getOrCreateConnection(const Engine::Net::IPv4Address& addr) -> AddConnRes {
		Connection* conn;
		Engine::ECS::Entity ent;
		const auto found = ipToPlayer.find(addr);
		if (found == ipToPlayer.end()) {
			auto& [e, c] = addConnection2(addr);
			ent = e;
			conn = &c;
		} else {
			ent = found->second;
			conn = world.getComponent<ConnectionComponent>(ent).conn.get();
		}
		return {ent, *conn};
	}

	void NetworkingSystem::broadcastDiscover() {
		#if ENGINE_CLIENT
			const auto now = Engine::Clock::now();
			for (auto it = servers.begin(); it != servers.end(); ++it) {
				if (it->second.lastUpdate + std::chrono::seconds{5} < now) {
					servers.erase(it);
				}
			}

			auto& [ent, conn] = getOrCreateConnection(group);
			conn.msgBegin<MessageType::DISCOVER_SERVER>();
			conn.write(DISCOVER_SERVER_DATA);
			conn.msgEnd<MessageType::DISCOVER_SERVER>();
		#endif
	}

	void NetworkingSystem::run(float32 dt) {
		now = Engine::Clock::now();

		// Recv messages
		int32 sz;
		while ((sz = socket.recv(&packet, sizeof(packet), address)) > -1) {
			auto& [ent, conn] = getOrCreateConnection(address);
			if (!conn.recv(packet, sz, now)) { continue; }

			const Engine::Net::MessageHeader* hdr; 
			while (hdr = conn.recvNext()) {
				dispatchMessage(ent, conn, hdr);
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

			for (auto& ent : connFilter) { // TODO: time is connections. reliable is plys
				auto& conn = *world.getComponent<ConnectionComponent>(ent).conn;
				const auto diff = now - conn.recvTime();
				if (diff > timeout) {
					ENGINE_LOG("Connection for ", ent ," (", conn.address(), ") timed out.");
					disconnect(ent);
					break; // Work around for not having an `it = container.erase(it)` alternative. Just check the rest next frame.
				}

				if (world.hasComponent<PlayerFlag>(ent)) {
					//conn.msgBegin<MessageType::ACK>();
					//conn.write(conn.getRecvNextAck());
					//conn.write(conn.getRecvAcks());
					//conn.msgEnd<MessageType::ACK>();

					// TODO: conn.sendUnacked(socket);
				}
			}
		}

		// Send Ack messages & unacked
		for (auto& ply : connFilter) {
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;
			conn.send(socket);
		}
	}

	void NetworkingSystem::runServer() {
		updateNeighbors();
		if (world.getAllComponentBitsets().size() > lastCompsBitsets.size()) {
			lastCompsBitsets.resize(world.getAllComponentBitsets().size());
		}

		for (auto& ply : plyFilter) {
			auto& neighComp = world.getComponent<NeighborsComponent>(ply);
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;

			// TODO: handle entities without phys comp?
			// TODO: figure out which entities have been updated
			// TODO: prioritize entities
			// TODO: figure out which comps on those entities have been updated

			for (const auto& pair : neighComp.addedNeighbors) {
				const auto& ent = pair.first;
				conn.msgBegin<MessageType::ECS_ENT_CREATE>(); // TODO: General_RO;
				conn.write(ent);
				conn.msgEnd<MessageType::ECS_ENT_CREATE>();

				ForEachIn<ComponentsSet>::call([&]<class C>() {
					if constexpr (IsNetworkedComponent<C>) {
						ENGINE_LOG("IsNetworkedComponent ", world.getComponentId<C>());
						if (!world.hasComponent<C>(ent)) { return; }

						auto& comp = world.getComponent<C>(ent);
						if (comp.netRepl() == Engine::Net::Replication::NONE) { return; }

						conn.msgBegin<MessageType::ECS_COMP_ADD>();//, General_RO);
						conn.write(ent);
						conn.write(world.getComponentId<C>());
						comp.netToInit(engine, world, ent, conn);
						conn.msgEnd<MessageType::ECS_COMP_ADD>();
					}
				});
			}

			for (const auto& pair : neighComp.removedNeighbors) {
				conn.msgBegin<MessageType::ECS_ENT_DESTROY>(); // TODO: General_RO;
				conn.write(pair.first);
				conn.msgEnd<MessageType::ECS_ENT_DESTROY>();
			}

			for (const auto& pair : neighComp.currentNeighbors) {
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
							// TODO: check if comp updated
							if (repl == Engine::Net::Replication::ALWAYS) {
								conn.msgBegin<MessageType::ECS_COMP_ALWAYS>(); //, General_UU);
								conn.write(ent);
								conn.write(cid);
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
					// TODO: we shouldnt have had to change the filters on all those systems because we are using ordered... Look into this.
					conn.msgBegin<MessageType::ECS_FLAG>(); // TODO: General_RO
					conn.write(ent);
					conn.write(flagComps);
					conn.msgEnd<MessageType::ECS_FLAG>();
				}
			}
		}

		lastCompsBitsets = world.getAllComponentBitsets();
	}

	
	void NetworkingSystem::runClient() {
		static uint8 ping = 0;
		static auto next = now;
		if (next > now) { return; }
		next = now + std::chrono::milliseconds{50};

		for (auto& ply : plyFilter) {
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;
			conn.msgBegin<MessageType::PING>();
			conn.write(static_cast<uint8>(++ping & 0x7F));
			conn.msgEnd<MessageType::PING>();
		}
	}

	int32 NetworkingSystem::connectionsCount() const {
		return static_cast<int32>(connFilter.size());
	}

	int32 NetworkingSystem::playerCount() const {
		return static_cast<int32>(plyFilter.size());
	}

	void NetworkingSystem::connectTo(const Engine::Net::IPv4Address& addr) {
		auto& [ent, conn] = getOrCreateConnection(addr);
		conn.msgBegin<MessageType::CONNECT>();
		conn.write(world.getTick());
		conn.msgEnd<MessageType::CONNECT>();
	}

	auto NetworkingSystem::addConnection2(const Engine::Net::IPv4Address& addr) -> AddConnRes {
		auto& ent = world.createEntity();
		ENGINE_INFO("Add connection: ", ent, " ", addr, " ", world.hasComponent<PlayerFlag>(ent), " ");
		ipToPlayer.emplace(addr, ent);
		auto& connComp = world.addComponent<ConnectionComponent>(ent);
		connComp.conn = std::make_unique<Connection>(addr, now);
		return {ent, *connComp.conn};
	}

	void NetworkingSystem::addPlayer(const Engine::ECS::Entity ent) {
		// TODO: i feel like this should be handled elsewhere. Where?

		ENGINE_INFO("Add player: ", ent, " ", world.hasComponent<PlayerFlag>(ent));

		if constexpr (ENGINE_SERVER) {
			auto& physSys = world.getSystem<PhysicsSystem>();
			world.setNetworked(ent, true);
			world.addComponent<PlayerFlag>(ent);
			world.addComponent<NeighborsComponent>(ent);
			world.addComponent<SpriteComponent>(ent).texture = engine.textureManager.get("assets/player.png");
			world.addComponent<PhysicsComponent>(ent).setBody(physSys.createPhysicsCircle(ent));
		}

		world.addComponent<ActionComponent>(ent);
		world.addComponent<MapEditComponent>(ent);
		world.addComponent<CharacterSpellComponent>(ent);
	}

	void NetworkingSystem::disconnect(Engine::ECS::Entity ent) {
		if (ent == Engine::ECS::INVALID_ENTITY) {
			return;
		}

		auto& conn = *world.getComponent<ConnectionComponent>(ent).conn;
		const auto addr = conn.address();
		ENGINE_LOG("Disconnecting ", ent, " ", addr);
		// TODO: really this should be a reliable message with timeout
		conn.msgBegin<MessageType::DISCONNECT>();
		conn.msgEnd<MessageType::DISCONNECT>();

		ipToPlayer.erase(addr);
		world.deferedDestroyEntity(ent);
	}

	void NetworkingSystem::dispatchMessage(Engine::ECS::Entity ent, Connection& from, const Engine::Net::MessageHeader* hdr) {
		constexpr auto msgToStr = [](const Engine::Net::MessageType& type) -> const char* {
			#define X(name) if (type == MessageType::name) { return #name; }
			#include <Game/MessageType.xpp>
			return "UNKNOWN";
		};

		//const auto* head = from.read<Engine::Net::MessageHeader>();
		//from.setMessageSize(head->size);
		//ENGINE_LOG("Read message: ", msgToStr(head->type), " ", head->channel, " ", head->size);
		//ENGINE_DEBUG_ASSERT(head != nullptr);

		// TODO: from unconnected players we only want to process connect and discover messages
		// TODO: impl
		//if (head->channel <= Engine::Net::Channel::ORDERED) {
		//	if (!from.reader.updateRecvAcks(*head)) {
		//		from.reader.read(head->size);
		//		return;
		//	}
		//}

		switch(hdr->type) {
			#define X(name) case MessageType::name: { handleMessageType<MessageType::name>(from, *hdr, ent); break; };
			#include <Game/MessageType.xpp>
			default: {
				ENGINE_WARN("Unhandled network message type ", static_cast<int32>(hdr->type));
			}
		}

		if constexpr (ENGINE_DEBUG) {
			const byte* stop = reinterpret_cast<const byte*>(hdr) + sizeof(*hdr) + hdr->size;
			const byte* curr = static_cast<const byte*>(from.read(0));
			const auto rem = stop - curr;

			// TODO: we should probably abort the whole packet if there is an error.
			if (rem > 0) {
				ENGINE_WARN("Incomplete read of network message ", msgToStr(hdr->type), " (", rem, " bytes remaining). Ignoring.");
				from.read(rem);
			} else if (rem < 0) {
				ENGINE_WARN("Read past end of network messge type ", msgToStr(hdr->type)," (", rem, " bytes remaining).");
			}
		}
	}

	void NetworkingSystem::updateNeighbors() {
		for (const auto ent : plyFilter) {
			auto& neighComp = world.getComponent<NeighborsComponent>(ent);
			auto& physComp = world.getComponent<PhysicsComponent>(ent);
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
					if (!world.isNetworked(ent)) { return true; }
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

			for (const auto lent : lastNeighbors) {
				if (!current.has(lent.first)) {
					removed.add(lent.first);
				}
			}

			for (const auto cent : current) {
				if (!lastNeighbors.has(cent.first)) {
					added.add(cent.first);
				}
			}

			// TODO: disabled vs destroyed
		}
	}
}
