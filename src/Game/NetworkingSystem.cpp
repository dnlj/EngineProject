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

		// TODO: This seems messy. Find better solution.
		msg.setSocket(socket);
		msg.setAddress(addr);
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
	void NetworkingSystem::handleMessageType<MessageType::PING>() {
		msg.read<Engine::Net::MessageHeader>(); // TODO: move into tick, handles probably dont care about this.
		if (msg.read<bool>()) {
			ENGINE_LOG("recv ping @ ", Engine::Clock::now().time_since_epoch().count() / 1E9);
			msg.reset();
			msg.next();
			msg.header().type = static_cast<uint8>(MessageType::PING);
			msg.write(false);
			msg.send();
		} else {
			ENGINE_LOG("recv pong @ ", Engine::Clock::now().time_since_epoch().count() / 1E9);
		}
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::TEST>() {
		puts("Test!");
	}

	void NetworkingSystem::tick(float32 dt) {
		if constexpr (ENGINE_CLIENT) {
			static auto next = world.getTickTime();
			if (next <= world.getTickTime()) {
				next = world.getTickTime() + std::chrono::seconds{1};
				msg.reset();
				msg.next();
				msg.header().type = static_cast<uint8>(MessageType::PING);
				msg.write(true);
				addr = {127,0,0,1, 27015}; // TODO: find a better way to setup MessageStream address/socket. This is far to error prone.
				msg.send();
			}
		}

		while (msg.recv() != -1) {
			auto& conn = getConnection(addr);
			conn.lastMessageTime = world.getTickTime();
			dispatchMessage();
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

	void NetworkingSystem::dispatchMessage() {
		#define HANDLE(Type) case Type: { return handleMessageType<Type>(); }
		switch(static_cast<MessageType>(msg.header().type)) {
			HANDLE(MessageType::PING);
			HANDLE(MessageType::TEST);
			default: {
				ENGINE_WARN("Unhandled network message type ", static_cast<int32>(msg.header().type));
			}
		}
		#undef HANDLE
	}
}
