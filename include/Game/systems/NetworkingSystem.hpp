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
#include <Game/Connection.hpp>


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

			Engine::Net::IPv4Address address;
			Engine::Net::Packet packet = {};
			const Engine::Net::IPv4Address group;
			Engine::Clock::TimePoint now = {};
			Engine::Clock::TimePoint lastUpdate = {};

			/** Main socket for talking to the server/clients */
			Engine::Net::UDPSocket socket;

			/** Used for multicast server discovery */
			#if ENGINE_SERVER
			Engine::Net::UDPSocket discoverServerSocket;
			#endif

			Engine::FlatHashMap<Engine::Net::IPv4Address, Engine::ECS::Entity> addressToEntity;

			pcg32 rng;
			uint16 genKey() {
				uint16 v;
				while (!(v = rng())) {}
				return v;
			} 

			// TODO: at some point we probably want to shrink this
			Engine::FlatHashMap<Engine::ECS::Entity, Engine::ECS::Entity> entToLocal;

		public:
			NetworkingSystem(SystemArg arg);
			void run(float32 dt);

			int32 connectionsCount() const; // TODO: remove or at least make private. Should interact with filter directly.
			int32 playerCount() const; // TODO: remove or at least make private. Should interact with filter directly.

			void connectTo(const Engine::Net::IPv4Address& addr);
			void requestDisconnect(const Engine::Net::IPv4Address& addr);

			auto& getSocket() noexcept { return socket; }

		private:
			void addPlayer(const Engine::ECS::Entity ent);
			Engine::ECS::Entity addConnection(const Engine::Net::IPv4Address& addr);
			Engine::ECS::Entity getEntity(const Engine::Net::IPv4Address& addr);
			Engine::ECS::Entity getOrCreateEntity(const Engine::Net::IPv4Address& addr);

			void recvAndDispatchMessages(Engine::Net::UDPSocket& sock);
			void dispatchMessage(Engine::ECS::Entity ent, ConnectionComponent& connComp, const Engine::Net::MessageHeader* hdr);
			void runClient();

			template<MessageType Type>
			void handleMessageType(Engine::ECS::Entity ent, ConnectionComponent& connComp, Connection& from, const Engine::Net::MessageHeader& head) {
				static_assert(Type != Type, "Unhandled network message type.");
			};
	};
}
