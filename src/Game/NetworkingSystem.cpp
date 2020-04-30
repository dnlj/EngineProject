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
	concept IsNetworkedComponent = requires (T t, Engine::Net::MessageStream& msg) {
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

	constexpr uint16 DEFAULT_PORT = 21212; // TODO: cmd line arg
	constexpr Engine::Net::IPv4Address MULTICAST_GROUP = {224,0,0,212, DEFAULT_PORT}; // TODO: cmd line arg

	// TODO: figure out a good pattern
	constexpr uint8 DISCOVER_SERVER_DATA[Engine::Net::MessageStream::MAX_MESSAGE_SIZE] = {
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
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, socket{ENGINE_SERVER ? DEFAULT_PORT : 0}
		, reader{socket}
		, anyConn{socket}
		, connFilter{world.getFilterFor<ConnectionComponent>()} {

		if (socket.setOption<Engine::Net::SocketOption::MULTICAST_JOIN>(MULTICAST_GROUP)) {
			ENGINE_LOG("LAN server discovery is available. Joining multicast group ", MULTICAST_GROUP);
		} else {
			ENGINE_WARN("LAN server discovery is unavailable; Unable to join multicast group ", MULTICAST_GROUP);
		}
	}

	void NetworkingSystem::setup() {
	}

	void NetworkingSystem::broadcastDiscover() {
		#if ENGINE_CLIENT
		{
			const auto now = Engine::Clock::now();
			for (auto it = servers.begin(); it != servers.end(); ++it) {
				if (it->second.lastUpdate + std::chrono::seconds{5} < now) {
					servers.erase(it);
				}
			}

			anyConn.writer.reset(MULTICAST_GROUP);
			anyConn.writer.next({static_cast<uint8>(MessageType::DISCOVER_SERVER)});
			anyConn.writer << DISCOVER_SERVER_DATA;
			anyConn.writer.send(); // TODO: test if getting on other systems
		}
		#endif
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::DISCOVER_SERVER>(Engine::Net::Connection& from) {
		constexpr auto size = sizeof(DISCOVER_SERVER_DATA);

		// TODO: rate limit per ip (longer if invalid packet)

		if (reader.size() == size && !memcmp(reader.current(), DISCOVER_SERVER_DATA, size)) {
			// TODO: rm - from.writer.reset(from);
			from.writer.next({static_cast<uint8>(MessageType::SERVER_INFO)});
			from.writer << "This is the name of the server";
			// TODO: rm - from.writer.send();
		}

		reader.clear();
	}
	
	template<>
	void NetworkingSystem::handleMessageType<MessageType::SERVER_INFO>(Engine::Net::Connection& from) {
		#if ENGINE_CLIENT
			auto& info = servers[from.address];
			info.name = reader.read<std::string>();
			info.lastUpdate = Engine::Clock::now();
		#endif
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::CONNECT>(Engine::Net::Connection& from) {
		// TODO: since messages arent reliable do connect/disconnect really make any sense?
		ENGINE_LOG("MessageType::CONNECT from ", from.address);
		addConnection(from.address);
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::DISCONNECT>(Engine::Net::Connection& from) {
		ENGINE_LOG("MessageType::DISCONNECT from ", from.address);
		disconnect(from);
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::PING>(Engine::Net::Connection& from) {
		if (reader.read<bool>()) {
			ENGINE_LOG("recv ping @ ", Engine::Clock::now().time_since_epoch().count() / 1E9, " from ", from.address);
			from.writer.next({static_cast<uint8>(MessageType::PING)});
			from.writer.write(false);
		} else {
			ENGINE_LOG("recv pong @ ", Engine::Clock::now().time_since_epoch().count() / 1E9, " from ", from.address);
		}
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ECS_COMP>(Engine::Net::Connection& from) {
		const auto ent = reader.read<Engine::ECS::Entity>();
		const auto cid = reader.read<Engine::ECS::ComponentId>();
		world.callWithComponent(ent, cid, [&](auto& comp){
			if constexpr (IsNetworkedComponent<decltype(comp)>) {
				comp.fromNetwork(reader);
			}
		});
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ACTION>(Engine::Net::Connection& from) {
		const auto aid = reader.read<Engine::Input::ActionId>();
		const auto val = reader.read<Engine::Input::Value>();
		std::cout << "Recv action: " << aid << " - " << val.value << "\n";
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
				next = now + std::chrono::seconds{2};

				for (auto& ply : connFilter) {
					auto& writer = world.getComponent<ConnectionComponent>(ply).conn->writer;
					writer.next({static_cast<uint8>(MessageType::PING)});
					writer.write(true);
				}
			}
		}

		while (reader.recv() > -1) {
			const auto& addr = reader.address();
			// TODO: dont connect withotu connect message
			auto found = ipToPlayer.find(addr);
			Engine::Net::Connection* conn;
			if (found != ipToPlayer.end()) {
				conn = &*world.getComponent<ConnectionComponent>(found->second).conn;
				conn->lastMessageTime = now;
			} else {
				if constexpr (ENGINE_CLIENT) { break; }

				anyConn.writer.flush();
				anyConn.address = addr;
				anyConn.writer.reset(addr);
				conn = &anyConn;
			}

			while (reader.size()) {
				dispatchMessage(*conn);
			}
		}

		anyConn.writer.flush();
		for (auto& ply : connFilter) {
			world.getComponent<ConnectionComponent>(ply).conn->writer.flush();
		}

		for (auto& ply : connFilter) { // TODO: merge into above loop
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
		anyConn.address = addr;
		anyConn.writer.reset(addr);
		anyConn.writer.next({static_cast<uint8>(MessageType::CONNECT)}); // TODO: reliable message
		anyConn.writer.send();
		addConnection(addr);
	}

	void NetworkingSystem::addConnection(const Engine::Net::IPv4Address& addr) {
		// TODO: i feel like this should be handled elsewhere. Where?
		auto& ply = world.createEntity();
		std::cout << "Created player: " << ply << "\n";
		ipToPlayer.emplace(addr, ply);
		auto& connComp = world.addComponent<ConnectionComponent>(ply); // TODO: setup socket for writer
		connComp.conn = std::make_unique<Engine::Net::Connection>(socket, addr, Engine::Clock::now(), 0);

		if constexpr (ENGINE_SERVER) {
			// TODO: world.addComponent<PlayerComponent>(ply);
			world.addComponent<MapEditComponent>(ply);
			world.addComponent<SpriteComponent>(ply).texture = engine.textureManager.get("../assets/player.png");
			//world.addComponent<PhysicsComponent>(ply).setBody(createPhysicsCircle(ply, physSys));
			world.addComponent<CharacterMovementComponent>(ply);
			world.addComponent<CharacterSpellComponent>(ply);
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
			<< "onDisconnect from: " << conn.address
			<< " @ " << conn.lastMessageTime.time_since_epoch().count()
			<< "\n";
	}

	void NetworkingSystem::dispatchMessage(Engine::Net::Connection& from) {
		// TODO: use array?
		const auto header = reader.read<Engine::Net::MessageHeader>();
		#define HANDLE(Type) case Type: { return handleMessageType<Type>(from); }
		switch(static_cast<MessageType>(header.type)) {
			HANDLE(MessageType::DISCOVER_SERVER);
			HANDLE(MessageType::SERVER_INFO);
			HANDLE(MessageType::CONNECT);
			HANDLE(MessageType::DISCONNECT);
			HANDLE(MessageType::PING);
			HANDLE(MessageType::ECS_COMP);
			HANDLE(MessageType::ACTION);
			default: {
				ENGINE_WARN("Unhandled network message type ", static_cast<int32>(reader.header().type));
			}
		}
		#undef HANDLE
	}
}
