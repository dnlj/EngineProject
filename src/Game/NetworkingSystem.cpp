// STD

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ECS/Entity.hpp>

// Game
#include <Game/World.hpp>
#include <Game/NetworkingSystem.hpp>

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

	constexpr uint16 DEFAULT_PORT = 27015; // TODO: cmd line arg
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
		, writer{socket} {

		std::cout << "Setting multicast: "
			<< socket.setOption<Engine::Net::SocketOption::MULTICAST_JOIN>(MULTICAST_GROUP)
			<< "\n";
	}

	void NetworkingSystem::setup() {
	}

	void NetworkingSystem::broadcastDiscover() {
		const auto now = Engine::Clock::now();
		for (auto it = servers.begin(); it != servers.end(); ++it) {
			if (it->second.lastUpdate + std::chrono::seconds{5} < now) {
				servers.erase(it);
			}
		}

		writer.reset(MULTICAST_GROUP);
		writer.next({static_cast<uint8>(MessageType::DISCOVER_SERVER)});
		writer << DISCOVER_SERVER_DATA;
		writer.send(); // TODO: test if getting on other systems
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::DISCOVER_SERVER>(const Engine::Net::IPv4Address& from) {
		constexpr auto size = sizeof(DISCOVER_SERVER_DATA);

		// TODO: rate limit per ip (longer if invalid packet)

		if (reader.size() == size && !memcmp(reader.current(), DISCOVER_SERVER_DATA, size)) {
			writer.reset(from);
			writer.next({static_cast<uint8>(MessageType::SERVER_INFO)});
			writer << "This is the name of the server";
			writer.send();
		}

		reader.clear();
	}
	
	template<>
	void NetworkingSystem::handleMessageType<MessageType::SERVER_INFO>(const Engine::Net::IPv4Address& from) {
		auto& info = servers[from];
		info.name = reader.read<std::string>();
		info.lastUpdate = Engine::Clock::now();
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::CONNECT>(const Engine::Net::IPv4Address& from) {
		// TODO: since messages arent reliable do connect/disconnect really make any sense?
		ENGINE_LOG("MessageType::CONNECT from ", from);
		//connect(from);
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::DISCONNECT>(const Engine::Net::IPv4Address& from) {
		ENGINE_LOG("MessageType::DISCONNECT from ", from);
		disconnect(from);
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::PING>(const Engine::Net::IPv4Address& from) {
		if (reader.read<bool>()) {
			ENGINE_LOG("recv ping @ ", Engine::Clock::now().time_since_epoch().count() / 1E9, " from ", from);
			writer.reset(from); // TODO: rm. shouldnt have to do this.
			writer.next({static_cast<uint8>(MessageType::PING)});
			writer.write(false);
			writer.send(); // TODO: rm. shouldnt have to do this.
		} else {
			ENGINE_LOG("recv pong @ ", Engine::Clock::now().time_since_epoch().count() / 1E9, " from ", from);
		}
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ECS_COMP>(const Engine::Net::IPv4Address& from) {
		const auto ent = reader.read<Engine::ECS::Entity>();
		const auto cid = reader.read<Engine::ECS::ComponentId>();
		world.callWithComponent(ent, cid, [&](auto& comp){
			if constexpr (IsNetworkedComponent<decltype(comp)>) {
				comp.fromNetwork(reader);
			}
		});
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

				writer.clear();
				writer.next({static_cast<uint8>(MessageType::PING)});
				writer.write(true);

				for (auto& [_, conn] : connections) {
					writer.sendto(conn.address);
				}

				writer.clear();
			}
		}

		while (reader.recv() > -1) {
			const auto& addr = reader.address();
			auto& conn = getConnection(addr);
			conn.lastMessageTime = now;
			while (reader.size()) {
				dispatchMessage(addr);
			}
		}

		// TODO: would probably be easiest for each connection to have its own reader/writer. That would allow use to combine single messages like PING.
		writer.flush();

		// TODO: Handle connection timeout
		for (auto it = connections.begin(); it != connections.end();) {
			auto& conn = it->second;
			const auto diff = now - conn.lastMessageTime;
			if (diff > timeout) {
				onDisconnect(conn);
				// TODO: send timeout message
				it = connections.erase(it);
			} else {
				++it;
			}
		}
	}

	int32 NetworkingSystem::connectionsCount() const {
		return static_cast<int32>(connections.size());
	}

	void NetworkingSystem::connect(const Engine::Net::IPv4Address& addr) {
		getConnection(addr);
	}

	void NetworkingSystem::disconnect(const Engine::Net::IPv4Address& addr) {
		auto found = connections.find(addr);
		if (found != connections.end()) {
			ENGINE_LOG("Disconnecting ", addr);
			onDisconnect(found->second);
			connections.erase(found);
		} else {
			ENGINE_LOG("Disconnecting ", addr, " (invalid)");
		}
	}

	void NetworkingSystem::onConnect(const Engine::Net::Connection& conn) {
		writer.reset(conn.address);
		writer.next({static_cast<uint8>(MessageType::CONNECT)});
		writer.send();
	}

	void NetworkingSystem::onDisconnect(const Engine::Net::Connection& conn) {
		std::cout
			<< "onDisconnect from: " << conn.address
			<< " @ " << conn.lastMessageTime.time_since_epoch().count()
			<< "\n";
	}

	Engine::Net::Connection& NetworkingSystem::getConnection(const Engine::Net::IPv4Address& addr) {
		auto found = connections.find(addr);
		if (found == connections.end()) {
			found = connections.emplace(
				addr,
				Engine::Net::Connection{addr, Engine::Clock::now(), 0}
			).first;
			onConnect(found->second);
		}
		return found->second;
	}

	void NetworkingSystem::dispatchMessage(const Engine::Net::IPv4Address& from) {
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
			default: {
				ENGINE_WARN("Unhandled network message type ", static_cast<int32>(reader.header().type));
			}
		}
		#undef HANDLE
	}
}
