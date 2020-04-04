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

	void NetworkingSystem::tick(float32 dt) {
		if constexpr (ENGINE_SERVER) {
			Engine::Net::IPv4Address addr;
			const auto size = socket.recv(msg.data(), msg.capacity(), addr);

			if (size != -1) {
				auto& conn = connections[addr];
				if (conn.lastMessageTime.time_since_epoch().count() == 0) {
					std::cout << "New connection from: " << addr << "\n";
				}

				conn.lastMessageTime = world.getTickTime();

				msg.reset(size);
				std::cout << addr << " - "
					<< msg.read<std::string>()
					<< Engine::Clock::Seconds{msg.read<Engine::Clock::TimePoint>().time_since_epoch()}.count() << "s"
					<< "\n";
				msg.reset();
				msg << "This is the response!";
				socket.send(addr, msg.data(), msg.size());
			}

			// TODO: Handle connection timeout
		} else {
			Engine::Net::IPv4Address addr = {127,0,0,1, 27015};
			msg.reset();
			msg << "This is a test message! " << Engine::Clock::now();
			auto size = socket.send(addr, msg.data(), msg.size());

			while (true) {
				const auto size = socket.recv(msg.data(), msg.capacity(), addr);
				if (size == -1) { break; }
				msg.reset(size);
				std::cout << "Received message from " << addr << " of size " << size << "\n";
			}
		}
		//Sleep(1000);
	}

	void NetworkingSystem::run(float32 dt) {
	}
}
