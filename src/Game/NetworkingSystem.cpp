// STD

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ECS/Entity.hpp>

// Game
#include <Game/World.hpp>
#include <Game/NetworkingSystem.hpp>
#include <Game/ConnectionComponent.hpp>

namespace {
	template<class T>
	concept IsNetworkedComponent = requires (T t, Engine::Net::Connection& msg) {
		t.toNetwork(msg);
		t.fromNetwork(msg);
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
	constexpr uint8 DISCOVER_SERVER_DATA[Engine::Net::Connection::MAX_MESSAGE_SIZE] = {
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
	void NetworkingSystem::handleMessageType<MessageType::DISCOVER_SERVER>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		constexpr auto size = sizeof(DISCOVER_SERVER_DATA);

		// TODO: rate limit per ip (longer if invalid packet)

		if (reader.size() == size && !memcmp(reader.current(), DISCOVER_SERVER_DATA, size)) {
			from.next(MessageType::SERVER_INFO, Engine::Net::Channel::UNRELIABLE);
			from << "This is the name of the server";
		}

		reader.clear();
	}
	
	template<>
	void NetworkingSystem::handleMessageType<MessageType::SERVER_INFO>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		#if ENGINE_CLIENT
			auto& info = servers[from.address()];
			info.name = reader.read<char*>();
			info.lastUpdate = Engine::Clock::now();
		#endif
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::CONNECT>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		// TODO: since messages arent reliable do connect/disconnect really make any sense?
		ENGINE_LOG("MessageType::CONNECT from ", from.address());
		addConnection(from.address());
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::DISCONNECT>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		ENGINE_LOG("MessageType::DISCONNECT from ", from.address());
		disconnect(from);
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::PING>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		if (reader.read<bool>()) {
			ENGINE_LOG("recv ping @ ", Engine::Clock::now().time_since_epoch().count() / 1E9, " from ", from.address());
			if (from.next(MessageType::PING, Engine::Net::Channel::RELIABLE)) {
				from.write(false);
			} else {
				ENGINE_WARN("TODO: how to handle unsendable messages");
			}
		} else {
			ENGINE_LOG("recv pong @ ", Engine::Clock::now().time_since_epoch().count() / 1E9, " from ", from.address());
		}
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ECS_COMP>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		//const auto ent = reader.read<Engine::ECS::Entity>();
		//const auto cid = reader.read<Engine::ECS::ComponentId>();
		//world.callWithComponent(ent, cid, [&](auto& comp){
		//	if constexpr (IsNetworkedComponent<decltype(comp)>) {
		//		comp.fromNetwork(reader);
		//	}
		//});
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ACTION>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		world.getSystem<ActionSystem>().processAction({
			ent,
			reader.read<Engine::Input::ActionId>(),
			reader.read<Engine::Input::Value>()
		});
		//std::cout << "Recv action: " << ent << " - " << aid << " - " << val.value << "\n";
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ACK>(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
		const auto& chan = reader.read<Engine::Net::Channel>();
		const auto& next = reader.read<Engine::Net::SequenceNumber>();
		const auto& acks = reader.read<uint64>();

		if (chan > Engine::Net::Channel::ORDERED) {
			ENGINE_WARN("Received ACK message for invalid channel ", static_cast<int32>(head.channel));
			return;
		}

		from.updateSentAcks(chan, next, acks);
	}
}


namespace Game {
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, socket{ENGINE_SERVER ? *engine.commandLineArgs.get<uint16>("port") : 0}
		, reader{socket}
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

			anyConn.reset(group);
			anyConn.next(MessageType::DISCOVER_SERVER, Engine::Net::Channel::UNRELIABLE);
			anyConn << DISCOVER_SERVER_DATA;
			anyConn.send(); // TODO: test if getting on other systems
		#endif
	}

	void NetworkingSystem::tick(float32 dt) {
		const auto now = Engine::Clock::now();

		if constexpr (ENGINE_SERVER) {
			/*for (const auto& conn : connections) {
				addr = conn.address;
				writer.reset();
				ForEachIn<ComponentsSet>::call([&]<class C>(){
					if constexpr (IsNetworkedComponent<C>) {
						for (const auto ent : world.getFilterFor<C>()) {
							writer.next({static_cast<uint8>(MessageType::ECS_COMP)});
							writer.write(ent);
							writer.write(world.getComponentId<C>());
							world.getComponent<C>(ent).toNetwork(writer);
						}
					}
				});
				writer.flush();
			}*/
		}

		if constexpr (ENGINE_CLIENT) {
			static auto next = now;
			if (next <= now) {
				next = now + std::chrono::seconds{1};

				for (auto& ply : connFilter) {
					auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;
					if (conn.next(MessageType::PING, Engine::Net::Channel::RELIABLE)) {
						conn.write(true);
					} else {
						ENGINE_WARN("TODO: how to handle unsendable messages");
					}
				}
			}
		}

		while (reader.recv() > -1) {
			const auto& addr = reader.address();
			auto found = ipToPlayer.find(addr);
			Engine::Net::Connection* conn;
			Engine::ECS::Entity ent;

			if (found != ipToPlayer.end()) {
				ent = found->second;
				conn = &*world.getComponent<ConnectionComponent>(ent).conn;
				conn->lastMessageTime = now;
			} else {
				ent = Engine::ECS::INVALID_ENTITY;
				anyConn.flush();
				anyConn.reset(addr);
				conn = &anyConn;
			}

			while (reader.size()) {
				dispatchMessage(ent, *conn);
			}
		}

		anyConn.flush();
		for (auto& ply : connFilter) {
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;
			conn.next(MessageType::ACK, Engine::Net::Channel::UNRELIABLE);
			// TODO: all channels
			conn.writeRecvAcks(Engine::Net::Channel::RELIABLE);

			// TODO: re-write unacked messages every x frames (seconds?)
			conn.writeUnacked(Engine::Net::Channel::RELIABLE);

			conn.flush();
		}

		for (auto& ply : connFilter) {
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;
			const auto diff = now - conn.lastMessageTime;
			if (diff > timeout) {
				onDisconnect(conn);
				// TODO: send timeout message
				world.destroyEntity(ply);
				break; // Work around for not having an `it = container.erase(it)` alternative. Just check the rest next frame.
			}
		}
	}

	int32 NetworkingSystem::connectionsCount() const {
		return static_cast<int32>(connFilter.size());
	}

	void NetworkingSystem::connectTo(const Engine::Net::IPv4Address& addr) {
		// TODO: reliable over anyConn doesnt make sense
		anyConn.reset(addr);
		std::cout << "Connect: " << anyConn.next(MessageType::CONNECT, Engine::Net::Channel::RELIABLE) << "\n";
		anyConn.send();
		addConnection(addr);	
	}

	void NetworkingSystem::addConnection(const Engine::Net::IPv4Address& addr) {
		// TODO: i feel like this should be handled elsewhere. Where?
		auto& ply = world.createEntity();
		ipToPlayer.emplace(addr, ply);
		auto& connComp = world.addComponent<ConnectionComponent>(ply);
		connComp.conn = std::make_unique<Engine::Net::Connection>(socket, addr, Engine::Clock::now());

		if constexpr (ENGINE_SERVER) {
			auto& physSys = world.getSystem<PhysicsSystem>();
			world.addComponent<PlayerComponent>(ply);
			world.addComponent<MapEditComponent>(ply);
			world.addComponent<SpriteComponent>(ply).texture = engine.textureManager.get("../assets/player.png");
			world.addComponent<PhysicsComponent>(ply).setBody(physSys.createPhysicsCircle(ply));
			world.addComponent<CharacterMovementComponent>(ply);
			world.addComponent<CharacterSpellComponent>(ply);
			world.addComponent<ActionComponent>(ply).grow(world.getSystem<ActionSystem>().count());
		}

		if constexpr (ENGINE_CLIENT) {
			ENGINE_DEBUG_ASSERT(ipToPlayer.size() == 1, "A Client should not be connected to more than one server.");
		}
	}

	void NetworkingSystem::disconnect(const Engine::Net::Connection& conn) {
		ENGINE_WARN("TODO: impl disconnect");
		//auto found = connections.find(addr);
		//if (found != connections.end()) {
		//	ENGINE_LOG("Disconnecting ", addr);
		//	onDisconnect(found->second);
		//	connections.erase(found);
		//} else {
		//	ENGINE_LOG("Disconnecting ", addr, " (invalid)");
		//}
	}

	void NetworkingSystem::onDisconnect(const Engine::Net::Connection& conn) {
		std::cout
			<< "onDisconnect from: " << conn.address()
			<< " @ " << conn.lastMessageTime.time_since_epoch().count()
			<< "\n";
	}

	void NetworkingSystem::dispatchMessage(Engine::ECS::Entity ent, Engine::Net::Connection& from) {
		const auto& head = reader.read<Engine::Net::MessageHeader>();

		// TODO: beter check for this
		if (head.channel != Engine::Net::Channel::UNRELIABLE) {
			if (!from.updateRecvAcks(head)) {
				ENGINE_WARN("TODO: Handle bad seq num");
				// TODO: this messages should be discarded. How to do with current write/read scheme?
				return;
			}
		}

		// TODO: use array?
		#define HANDLE(Type) case Type: { return handleMessageType<Type>(from, head, ent); }
		switch(head.type) {
			HANDLE(MessageType::DISCOVER_SERVER);
			HANDLE(MessageType::SERVER_INFO);
			HANDLE(MessageType::CONNECT);
			HANDLE(MessageType::DISCONNECT);
			HANDLE(MessageType::PING);
			HANDLE(MessageType::ECS_COMP);
			HANDLE(MessageType::ACTION);
			HANDLE(MessageType::ACK);
			default: {
				ENGINE_WARN("Unhandled network message type ", static_cast<int32>(head.type));
			}
		}
		#undef HANDLE
	}
}
