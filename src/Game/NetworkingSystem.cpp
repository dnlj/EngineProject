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
}


namespace Game {
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, socket{ENGINE_SERVER ? 27015 : 0} {

		// TODO: This seems messy. Find better solution.
		writer.setSocket(socket);
		writer.setAddress(addr);
		reader.setSocket(socket);
		reader.setAddress(addr);
	}

	void NetworkingSystem::setup() {
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::PING>() {
		if (reader.read<bool>()) {
			ENGINE_LOG("recv ping @ ", Engine::Clock::now().time_since_epoch().count() / 1E9);
			writer.reset(); // TODO: rm. shouldnt have to do this.
			writer.next({static_cast<uint8>(MessageType::PING)});
			writer.write(false);
			writer.send(); // TODO: rm. shouldnt have to do this.
		} else {
			ENGINE_LOG("recv pong @ ", Engine::Clock::now().time_since_epoch().count() / 1E9);
		}
	}

	template<>
	void NetworkingSystem::handleMessageType<MessageType::ECS_COMP>() {
		const auto ent = reader.read<Engine::ECS::Entity>();
		const auto cid = reader.read<Engine::ECS::ComponentId>();
		world.callWithComponent(ent, cid, [&](auto& comp){
			if constexpr (IsNetworkedComponent<decltype(comp)>) {
				comp.fromNetwork(reader);
			}
		});
	}

	void NetworkingSystem::tick(float32 dt) {
		if constexpr (ENGINE_CLIENT) {
			static auto next = world.getTickTime();
			if (next <= Engine::Clock::now()) {
				next = Engine::Clock::now() + std::chrono::seconds{1};
				writer.reset();
				writer.next({static_cast<uint8>(MessageType::PING)});
				writer.write(true);
				addr = {127,0,0,1, 27015}; // TODO: find a better way to setup MessageStream address/socket. This is far to error prone.
				writer.send();
			}
		} else {
			for (const auto& conn : connections) {
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
				if (writer.size() > 0) {
					writer.send();
				}
			}
		}

		while (reader.recv() > -1) {
			auto& conn = getConnection(addr);
			conn.lastMessageTime = world.getTickTime();
			while (reader.size()) {
				dispatchMessage();
			}
		}

		if (writer.size() > 0) {
			writer.send();
		}

		// TODO: Handle connection timeout
	}

	Engine::Net::Connection& NetworkingSystem::getConnection(const Engine::Net::IPv4Address& addr) {
		const auto [found, ins] = ipToConnection.emplace(addr, static_cast<uint8>(connections.size()));
		if (ins) {
			connections.emplace_back(addr, Engine::Clock::now(), 0);
		}
		return connections[found->second];
	}

	void NetworkingSystem::dispatchMessage() {
		// TODO: use array?
		const auto header = reader.read<Engine::Net::MessageHeader>();
		#define HANDLE(Type) case Type: { return handleMessageType<Type>(); }
		switch(static_cast<MessageType>(header.type)) {
			HANDLE(MessageType::PING);
			HANDLE(MessageType::ECS_COMP);
			default: {
				ENGINE_WARN("Unhandled network message type ", static_cast<int32>(reader.header().type));
			}
		}
		#undef HANDLE
	}
}
