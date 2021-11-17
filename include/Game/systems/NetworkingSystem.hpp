#pragma once

// PCG
#include <pcg_random.hpp>

// Engine
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/Connection.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Engine.hpp>
#include <Engine/ECS/Common.hpp>

// Game
#include <Game/System.hpp>
#include <Game/MessageType.hpp>


namespace Game {
	class ConnectionComponent;

	class NetworkingSystem : public System {
		public:
			#if ENGINE_CLIENT
				// TODO: find better way to handle this
				struct ServerInfo {
					Engine::Clock::TimePoint lastUpdate;
					std::string name;
					// TODO: name, player count, ping, private, tags, etc.
				};
				Engine::FlatHashMap<Engine::Net::IPv4Address, ServerInfo> servers;
				void broadcastDiscover();
			#endif

		private:
			static constexpr auto timeout = std::chrono::milliseconds{5000};
			static constexpr auto disconnectTime = std::chrono::milliseconds{500};
			Engine::Net::UDPSocket socket;

			// TODO: should this be part of Connection?
			struct ConnInfo {
				Engine::ECS::Entity ent = {};
			};
			Engine::FlatHashMap<Engine::Net::IPv4Address, ConnInfo> connections; // TODO: name

			pcg32 rng;
			uint16 genKey() {
				uint16 v;
				while (!(v = rng())) {}
				return v;
			} 
			Engine::Net::IPv4Address address;
			Engine::Net::Packet packet = {};
			const Engine::Net::IPv4Address group;
			Engine::Clock::TimePoint now = {};
			Engine::Clock::TimePoint lastUpdate = {};

			// TODO: at some point we probably want to shrink this
			Engine::FlatHashMap<Engine::ECS::Entity, Engine::ECS::Entity> entToLocal;

		public:
			NetworkingSystem(SystemArg arg);
			void run(float32 dt);
			int32 connectionsCount() const;
			int32 playerCount() const;
			void connectTo(const Engine::Net::IPv4Address& addr);
			void requestDisconnect(const Engine::Net::IPv4Address& addr);

			auto& getSocket() noexcept { return socket; }

		private:
			void addPlayer(const Engine::ECS::Entity ent);
			Engine::ECS::Entity addConnection2(const Engine::Net::IPv4Address& addr); // TODO: rename
			Engine::ECS::Entity getOrCreateConnection(const Engine::Net::IPv4Address& addr); // TODO: rename to map address or similar

			void dispatchMessage(Engine::ECS::Entity ent, ConnectionComponent& connComp, const Engine::Net::MessageHeader* hdr);
			void runClient();

			template<MessageType Type>
			void handleMessageType(Engine::ECS::Entity ent, ConnectionComponent& connComp, Connection& from, const Engine::Net::MessageHeader& head) {
				static_assert(Type != Type, "Unhandled network message type.");
			};
	};
}
