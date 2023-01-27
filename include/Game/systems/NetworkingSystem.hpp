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
	class ConnectionInfo : public Connection {
		public:
			using Connection::Connection;
			Engine::ECS::Entity ent{}; // TODO: probably add getter/setter once conversion is done. Makes access cleaner
			Engine::Clock::TimePoint disconnectAt = {};
	};

	using NetworkMessageHandler = void(*)(EngineInstance& engine, ConnectionInfo& from, const Engine::Net::MessageHeader hdr, Engine::Net::BufferReader& msg);

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
			static constexpr auto timeout = std::chrono::milliseconds{5000}; // TODO: Should be configurable
			static constexpr auto disconnectingPeriod = std::chrono::milliseconds{500}; // TODO: Should be configurable

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

			Engine::FlatHashMap<Engine::Net::IPv4Address, std::unique_ptr<ConnectionInfo>> addrToConn;
			Engine::FlatHashMap<Engine::ECS::Entity, ConnectionInfo*> entToConn;
			
			pcg32 rng;
			uint16 genKey() {
				uint16 v;
				while (!(v = rng())) {}
				return v;
			} 


		public:
			NetworkingSystem(SystemArg arg);
			void update(float32 dt);

			int32 playerCount() const; // TODO: remove or at least make private. Should interact with filter directly.

			// TODO: make client only
			void connectTo(const Engine::Net::IPv4Address& addr);

			/**
			 * Attempt a graceful disconnect.
			 */
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

			void addPlayer(ConnectionInfo& conn); // TODO: move addPlayer to EntityNetworkingSystem?

			ConnectionInfo* getConnection(Engine::ECS::Entity ent) {
				auto found = entToConn.find(ent);
				return found == entToConn.end() ? nullptr : found->second;
			}

			ConnectionInfo* getConnection(Engine::Net::IPv4Address addr) {
				auto found = addrToConn.find(addr);
				return found == addrToConn.end() ? nullptr : found->second.get();
			}

			auto& getConnections() { return addrToConn; }
			const auto& getConnections() const { return addrToConn; }

		private:
			ConnectionInfo& getOrCreateConnection(const Engine::Net::IPv4Address& addr);

			/**
			 * Cleanup any ECS state associated with a connection.
			 */
			void cleanECS(ConnectionInfo& conn);

			/**
			 * Disconnects a connection/entity and schedules its destruction.
			 */
			void disconnect(ConnectionInfo& conn);

			void recvAndDispatchMessages(Engine::Net::UDPSocket& sock);
			void dispatchMessage(ConnectionInfo& from, const Engine::Net::MessageHeader hdr, Engine::Net::BufferReader& msg);

			template<MessageType Type>
			static void handleMessageType(EngineInstance& engine, ConnectionInfo& from, const Engine::Net::MessageHeader head, Engine::Net::BufferReader& msg) {
				static_assert(Type != Type, "Unhandled network message type.");
			};
	};
}
