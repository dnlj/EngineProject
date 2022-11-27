#pragma once

// PCG
#include <pcg_random.hpp>

// Engine
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/Connection.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Engine.hpp>
#include <Engine/ECS/ecs.hpp>

// Game
#include <Game/System.hpp>
#include <Game/MessageType.hpp>
#include <Game/Connection.hpp>


namespace Game {
	class ConnectionComponent;

	using NetworkMessageHandler = void(*)(EngineInstance& engine, Engine::ECS::Entity ent, Connection& from, const Engine::Net::MessageHeader hdr, Engine::Net::BufferReader& msg);

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

			NetworkMessageHandler msgHandlers[MessageType::_count] = {};

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


		public:
			NetworkingSystem(SystemArg arg);
			void update(float32 dt);

			int32 connectionsCount() const; // TODO: remove or at least make private. Should interact with filter directly.
			int32 playerCount() const; // TODO: remove or at least make private. Should interact with filter directly.

			void connectTo(const Engine::Net::IPv4Address& addr);
			void requestDisconnect(const Engine::Net::IPv4Address& addr);

			auto& getSocket() noexcept { return socket; }

			void setMessageHandler(MessageType msg, NetworkMessageHandler func) noexcept {
				if (msg >= MessageType::_count) {
					ENGINE_WARN("Invalid message type ", msg);
					ENGINE_DEBUG_ASSERT(false);
					return;
				}

				ENGINE_DEBUG_ASSERT(!msgHandlers[msg], "Attempting to overwrite message handler.");
				msgHandlers[msg] = func;
			}

			void addPlayer(const Engine::ECS::Entity ent); // TODO: move addPlayer to EntityNetworkingSystem

		private:
			Engine::ECS::Entity addConnection(const Engine::Net::IPv4Address& addr);
			Engine::ECS::Entity getEntity(const Engine::Net::IPv4Address& addr);
			Engine::ECS::Entity getOrCreateEntity(const Engine::Net::IPv4Address& addr);
			void disconnect(Engine::ECS::Entity ent, ConnectionComponent& connComp);

			void recvAndDispatchMessages(Engine::Net::UDPSocket& sock);
			void dispatchMessage(Engine::ECS::Entity ent, ConnectionComponent& connComp, const Engine::Net::MessageHeader hdr, Engine::Net::BufferReader& msg);
			void updateClient();

			template<MessageType Type>
			static void handleMessageType(EngineInstance& engine, Engine::ECS::Entity ent, Connection& from, const Engine::Net::MessageHeader head, Engine::Net::BufferReader& msg) {
				static_assert(Type != Type, "Unhandled network message type.");
			};
	};
}
