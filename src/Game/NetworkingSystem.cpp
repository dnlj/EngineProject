// STD

// Engine
#include <Engine/Clock.hpp>

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
}


namespace Game {
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, socket{ENGINE_SERVER ? 27015 : 0} {
	}

	void NetworkingSystem::setup() {
		ForEachIn<ComponentsSet>::call([&]<class C>(){
			if constexpr (IsNetworkedComponent<C>) {
				for (const auto eid : world.getFilterFor<C>()) {
					//msg.reset();
					//// TODO: move into some kind of header?
					//msg.write(eid);
					//msg.write(world.getComponentID<C>());
					//world.getComponent<C>(eid).toNetwork(msg);
				}
			}
		});
	}

	template<>
	void NetworkingSystem::handleMessage<Engine::ServerSide>(const Engine::Net::IPv4Address& from) {
		std::cout << from << " - "
			<< msg.read<std::string>()
			<< Engine::Clock::Seconds{msg.read<Engine::Clock::TimePoint>().time_since_epoch()}.count() << "s"
			<< "\n";
		msg.reset();
		msg << "This is the response!";
		socket.send(from, msg.data(), msg.size());
	}

	template<>
	void NetworkingSystem::handleMessage<Engine::ClientSide>(const Engine::Net::IPv4Address& from) {
		std::cout << "Received message from " << from << "\n";
	}

	void NetworkingSystem::tick(float32 dt) {
		Engine::Net::IPv4Address addr = {127,0,0,1, 27015};

		if constexpr (ENGINE_CLIENT) {
			msg.reset();
			msg << "This is a test message! " << Engine::Clock::now();
			socket.send(addr, msg.data(), msg.size());
		}

		while (true) {
			const auto size = socket.recv(msg.data(), msg.capacity(), addr);
			if (size == -1) { break; }
			auto& conn = getConnection(addr);
			conn.lastMessageTime = world.getTickTime();
			msg.reset(size);
			handleMessage<ENGINE_SIDE>(addr);
		}

		// TODO: Handle connection timeout
	}

	Engine::Net::Connection& NetworkingSystem::getConnection(const Engine::Net::IPv4Address& addr) {
		const auto [found, ins] = ipToConnection.emplace(addr, static_cast<uint8>(connections.size()));
		if (ins) {
			connections.emplace_back();
		}
		return connections[found->second];
	}
}
