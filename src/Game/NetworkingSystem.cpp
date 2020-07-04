// STD
#include <set>
#include <concepts>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ECS/Entity.hpp>

// Game
#include <Game/World.hpp>
#include <Game/NetworkingSystem.hpp>
#include <Game/ConnectionComponent.hpp>

namespace {
	template<class T>
	concept IsNetworkedComponent = requires (
			T t,
			Engine::Net::PacketWriter& writer,
			Engine::Net::PacketReader& reader,
			Game::World& world,
			Engine::ECS::Entity ent
		) {
		Engine::Net::Replication{t.netRepl()};
		t.netTo(writer);
		t.netToInit(world, ent, writer);
		t.netFrom(reader);
		t.netFromInit(world, ent, reader);
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
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
	};
}


namespace Game {
	template<>
	void NetworkingSystem::handleMessageType<MessageType::UNKNOWN>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::DISCOVER_SERVER>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		constexpr auto size = sizeof(DISCOVER_SERVER_DATA);

		// TODO: rate limit per ip (longer if invalid packet)

		if (from.reader.size() == size && !memcmp(from.reader.read(size), DISCOVER_SERVER_DATA, size)) {
			from.writer.next(MessageType::SERVER_INFO, Engine::Net::Channel::UNRELIABLE);
			from.writer.write("This is the name of the server");
		}
	}
	
	template<>
	void NetworkingSystem::handleMessageType<MessageType::SERVER_INFO>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		#if ENGINE_CLIENT
			auto& info = servers[from.address()];
			info.name = from.reader.read<char*>();
			info.lastUpdate = Engine::Clock::now();
		#endif
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::CONNECT>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		// TODO: since messages arent reliable do connect/disconnect really make any sense?
		ENGINE_LOG("MessageType::CONNECT from ", from.address());
		auto& conn = addConnection(from.address());
		conn.writer.next(MessageType::CONNECT_CONFIRM, Engine::Net::Channel::RELIABLE);
	}
	
	template<>
	void NetworkingSystem::handleMessageType<MessageType::CONNECT_CONFIRM>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		// TODO: server has acknowledged connection.
		ENGINE_WARN("TODO: Server has acknowledged connection.");
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::DISCONNECT>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		puts("MessageType::DISCONNECT");
		disconnect(ent);
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::PING>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		if (*from.reader.read<bool>()) {
			ENGINE_LOG("recv ping @ ", Engine::Clock::now().time_since_epoch().count() / 1E9, " from ", from.address());
			if (from.writer.next(MessageType::PING, Engine::Net::Channel::RELIABLE)) {
				from.writer.write(false);
			} else {
				ENGINE_WARN("TODO: how to handle unsendable messages");
			}
		} else {
			ENGINE_LOG("recv pong @ ", Engine::Clock::now().time_since_epoch().count() / 1E9, " from ", from.address());
		}
	}
	
	template<>
	void NetworkingSystem::handleMessageType<MessageType::ECS_ENT_CREATE>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		// TODO: this message should be client only
		auto* remote = from.reader.read<Engine::ECS::Entity>();
		if (!remote) { return; }

		auto& local = entToLocal[*remote];
		if (local == Engine::ECS::INVALID_ENTITY) {
			local = world.createEntity();
		}

		// TODO: components init
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ECS_ENT_DESTROY>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		// TODO: this message should be client only
		auto* remote = from.reader.read<Engine::ECS::Entity>();
		if (!remote) { return; }
		auto found = entToLocal.find(*remote);
		if (found != entToLocal.end()) {
			puts("ECS_ENT_DESTROY");
			world.destroyEntity(found->second);
			entToLocal.erase(found);
		}
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ECS_COMP_ADD>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		// TODO: this message should be client only
		const auto* remote = from.reader.read<Engine::ECS::Entity>();
		const auto* cid = from.reader.read<Engine::ECS::ComponentId>();
		if (!remote || !cid) { return; }

		auto found = entToLocal.find(*remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		world.callWithComponent(*cid, [&]<class C>(){
			if constexpr (IsNetworkedComponent<C>) {
				if (!world.hasComponent<C>(local)) {
					ENGINE_LOG("ECS_COMP_ADD ", local, " ", *cid);
					auto& comp = world.addComponent<C>(local);
					comp.netFromInit(world, local, from.reader);
				}
			} else {
				ENGINE_WARN("Attemping to network non-network component");
			}
		});
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ECS_COMP_ALWAYS>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		// TODO: this message should be client only
		const auto* remote = from.reader.read<Engine::ECS::Entity>();
		const auto* cid = from.reader.read<Engine::ECS::ComponentId>();
		if (!remote || !cid) { return; }

		auto found = entToLocal.find(*remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		// TODO: What if we get an older message after a newer message?
		world.callWithComponent(*cid, [&]<class C>(){
			if constexpr (IsNetworkedComponent<C>) {
				if (world.hasComponent<C>(local)) {
					world.getComponent<C>(local).netFrom(from.reader);
				}
			} else {
				ENGINE_WARN("Attemping to network non-network component");
			}
		});
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ECS_FLAG>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		// TODO: this message should be client only
		const auto remote = from.reader.read<Engine::ECS::Entity>();
		const auto flags = from.reader.read<Engine::ECS::ComponentBitset>();
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
				ENGINE_LOG("Remove component ", local, cid);
			} else {
				world.addComponent<C>(local);
				ENGINE_LOG("Add component ", local, cid);
			}
		});
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ACTION>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		const auto* aid = from.reader.read<Engine::Input::ActionId>();
		const auto* val = from.reader.read<Engine::Input::Value>();

		if (aid && val) {
			// TODO: sanity check inputs
			world.getSystem<ActionSystem>().processAction(ent, *aid, *val);
		}
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ACK>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		const auto& chan = *from.reader.read<Engine::Net::Channel>();
		const auto& next = *from.reader.read<Engine::Net::SequenceNumber>();
		const auto& acks = *from.reader.read<uint64>();

		if (chan > Engine::Net::Channel::ORDERED) {
			ENGINE_WARN("Received ACK message for invalid channel ", static_cast<int32>(head.channel));
			return;
		}

		from.writer.updateSentAcks(chan, next, acks);
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::TEST>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		std::cout << "***** TEST: " << head.sequence << "\n";
	}
}


namespace Game {
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, socket{ENGINE_SERVER ? *engine.commandLineArgs.get<uint16>("port") : 0}
		, anyConn{socket}
		, connFilter{world.getFilterFor<ConnectionComponent>()}
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

	void NetworkingSystem::broadcastDiscover() {
		#if ENGINE_CLIENT
			const auto now = Engine::Clock::now();
			for (auto it = servers.begin(); it != servers.end(); ++it) {
				if (it->second.lastUpdate + std::chrono::seconds{5} < now) {
					servers.erase(it);
				}
			}

			anyConn.writer.addr = group;
			anyConn.writer.next(MessageType::DISCOVER_SERVER, Engine::Net::Channel::UNRELIABLE);
			anyConn.writer.write(DISCOVER_SERVER_DATA);
			anyConn.writer.flush(); // TODO: test if getting on other systems
		#endif
	}

	// TODO: should be in update, not tick
	void NetworkingSystem::tick(float32 dt) {
		const auto now = Engine::Clock::now();

		if constexpr (ENGINE_SERVER) {
			if (world.getAllComponentBitsets().size() > lastCompsBitsets.size()) {
				lastCompsBitsets.resize(world.getAllComponentBitsets().size());
			}

			for (auto& ply : connFilter) {
				auto& neighComp = world.getComponent<NeighborsComponent>(ply);
				auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;

				// TODO: handle entities without phys comp?
				// TODO: figure out which entities have been updated
				// TODO: prioritize entities
				// TODO: figure out which comps on those entities have been updated

				for (const auto& pair : neighComp.addedNeighbors) {
					const auto& ent = pair.first;
					conn.writer.next(MessageType::ECS_ENT_CREATE, Engine::Net::Channel::ORDERED);
					conn.writer.write(ent);

					ForEachIn<ComponentsSet>::call([&]<class C>() {
						if constexpr (IsNetworkedComponent<C>) {
							conn.writer.next(MessageType::ECS_COMP_ADD, Engine::Net::Channel::ORDERED);
							conn.writer.write(ent);
							conn.writer.write(world.getComponentId<C>());
							world.getComponent<C>(ent).netToInit(world, ent, conn.writer);
						}
					});
				}

				for (const auto& pair : neighComp.removedNeighbors) {
					conn.writer.next(MessageType::ECS_ENT_DESTROY, Engine::Net::Channel::ORDERED);
					conn.writer.write(pair.first);
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
									conn.writer.next(MessageType::ECS_COMP_ALWAYS, Engine::Net::Channel::UNRELIABLE);
									conn.writer.write(ent);
									conn.writer.write(cid);
									comp.netTo(conn.writer);
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
						conn.writer.next(MessageType::ECS_FLAG, Engine::Net::Channel::ORDERED);
						conn.writer.write(ent);
						conn.writer.write(flagComps);
					}
				}
			}

			lastCompsBitsets = world.getAllComponentBitsets();
		}

		if constexpr (ENGINE_CLIENT) {
			static auto next = now;
			if (next <= now) {
				next = now + std::chrono::seconds{1};

				for (auto& ply : connFilter) {
					auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;
					if (conn.writer.next(MessageType::PING, Engine::Net::Channel::RELIABLE)) {
						conn.writer.write(true);
					} else {
						ENGINE_WARN("TODO: how to handle unsendable messages");
					}
				}

				//for (auto& ply : connFilter) {
				//	auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;
				//
				//	// TODO: test that mixing works using channel reliable
				//	for (int i = 0; i < 10; ++i) {
				//		conn.writer.flush();
				//		if (conn.writer.next(MessageType::TEST, Engine::Net::Channel::ORDERED)) {
				//			conn.writer.send();
				//		} else {
				//			ENGINE_WARN("TODO: how to handle unsendable messages");
				//		}
				//	}
				//}
			}
		}

		int32 sz;
		while ((sz = socket.recv(&packet, sizeof(packet), address)) > -1) {
			auto found = ipToPlayer.find(address);
			Engine::Net::Connection* conn;
			Engine::ECS::Entity ent;

			if (found != ipToPlayer.end()) {
				ent = found->second;
				conn = &*world.getComponent<ConnectionComponent>(ent).conn;
				conn->lastMessageTime = now;
			} else {
				ent = Engine::ECS::INVALID_ENTITY;
				anyConn.writer.flush();
				anyConn.writer.addr = address;
				conn = &anyConn;
			}

			sz -= sizeof(Engine::Net::PacketHeader);
			char* start = reinterpret_cast<char*>(&packet.data);
			conn->reader.set(start, start + sz);

			while (conn->reader.next()) {
				dispatchMessage(ent, *conn);
			}
		}

		anyConn.writer.flush();
		for (auto& ply : connFilter) {
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;

			for (Engine::Net::Channel ch{0}; ch <= Engine::Net::Channel::ORDERED; ++ch) {
				conn.writer.next(MessageType::ACK, Engine::Net::Channel::UNRELIABLE);
				conn.writeRecvAcks(ch);
				conn.writer.writeUnacked(ch); // TODO: re-write unacked messages only every x frames (seconds?)
			}

			conn.writer.flush();
		}

		for (auto& ply : connFilter) {
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;
			const auto diff = now - conn.lastMessageTime;
			if (diff > timeout) {
				std::cout << "Timeout: " << ply << "\n";
				disconnect(ply);
				break; // Work around for not having an `it = container.erase(it)` alternative. Just check the rest next frame.
			}
		}
	}

	int32 NetworkingSystem::connectionsCount() const {
		return static_cast<int32>(connFilter.size());
	}

	void NetworkingSystem::connectTo(const Engine::Net::IPv4Address& addr) {
		auto& conn = addConnection(addr);
		conn.writer.next(MessageType::CONNECT, Engine::Net::Channel::UNRELIABLE);
		conn.writer.flush();
	}

	Engine::Net::Connection& NetworkingSystem::addConnection(const Engine::Net::IPv4Address& addr) {
		// TODO: i feel like this should be handled elsewhere. Where?
		auto& ply = world.createEntity();
		ipToPlayer.emplace(addr, ply);
		auto& connComp = world.addComponent<ConnectionComponent>(ply);
		connComp.conn = std::make_unique<Engine::Net::Connection>(socket, addr, Engine::Clock::now());

		if constexpr (ENGINE_SERVER) {
			auto& physSys = world.getSystem<PhysicsSystem>();
			world.addComponent<PlayerFlag>(ply);
			world.addComponent<MapEditComponent>(ply);
			world.addComponent<SpriteComponent>(ply).texture = engine.textureManager.get("assets/player.png");
			world.addComponent<PhysicsComponent>(ply).setBody(physSys.createPhysicsCircle(ply));
			world.addComponent<CharacterMovementComponent>(ply);
			world.addComponent<CharacterSpellComponent>(ply);
			world.addComponent<ActionComponent>(ply).grow(world.getSystem<ActionSystem>().count());
			world.addComponent<NeighborsComponent>(ply);
		}

		if constexpr (ENGINE_CLIENT) {
			ENGINE_DEBUG_ASSERT(ipToPlayer.size() == 1, "A Client should not be connected to more than one server.");
		}

		return *connComp.conn;
	}

	void NetworkingSystem::disconnect(Engine::ECS::Entity ent) {
		if (ent == Engine::ECS::INVALID_ENTITY) {
			return;
		}

		auto& conn = *world.getComponent<ConnectionComponent>(ent).conn;
		const auto addr = conn.address();
		ENGINE_LOG("Disconnecting ", ent, " ", addr);
		conn.writer.flush();
		conn.writer.next(MessageType::DISCONNECT, Engine::Net::Channel::UNRELIABLE);
		conn.writer.flush();

		ipToPlayer.erase(addr);
		world.destroyEntity(ent);
	}

	void NetworkingSystem::dispatchMessage(Engine::ECS::Entity ent, Engine::Net::Connection& from) {
		const auto* head = from.reader.read<Engine::Net::MessageHeader>();
		ENGINE_DEBUG_ASSERT(head != nullptr);

		// TODO: from unconnected players we only want to process connect and discover messages
		if (head->channel <= Engine::Net::Channel::ORDERED) {
			if (!from.reader.updateRecvAcks(*head)) {
				from.reader.read(head->size);
				return;
			}
		}

		switch(head->type) {
			#define X(name) case MessageType::name: { handleMessageType<MessageType::name>(from, *head, ent); break; };
			#include <Game/MessageType.xpp>
			default: {
				ENGINE_WARN("Unhandled network message type ", static_cast<int32>(head->type));
			}
		}

		if constexpr (ENGINE_DEBUG) {
			const byte* stop = reinterpret_cast<const byte*>(head) + sizeof(*head) + head->size;
			const byte* curr = static_cast<const byte*>(from.reader.read(0));
			const auto rem = stop - curr;
			constexpr auto msgToStr = [](const Engine::Net::MessageType& type) -> const char* {
				#define X(name) if (type == MessageType::name) { return #name; }
				#include <Game/MessageType.xpp>
				return "UNKNOWN";
			};

			if (rem > 0) {
				ENGINE_WARN("Incomplete read of network message ", msgToStr(head->type), " (", rem, " bytes remaining). Ignoring.");
				from.reader.read(rem);
			} else if (rem < 0) {
				ENGINE_WARN("Network message read past message end (", rem, " bytes remaining).");
			}
		}
	}
}
