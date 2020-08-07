#pragma once

// Engine
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/Connection.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Engine.hpp>
#include <Engine/ECS/Common.hpp>

// Game
#include <Game/System.hpp>
#include <Game/EntityFilter.hpp>
#include <Game/MessageType.hpp>
#include <Game/comps/NeighborsComponent.hpp>


namespace Game {
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
			#endif

		private:
			static constexpr auto timeout = std::chrono::milliseconds{10'000};
			Engine::Net::UDPSocket socket;
			EntityFilter& connFilter;
			EntityFilter& plyFilter;

			// TODO: should this be part of Connection?
			//enum class ConnectionState {
				//
			//};
			struct ConnInfo {

				Engine::ECS::Entity ent = {};
				uint64 key = 0; // TODO: if we want to do this correctly we need to support it at a lower level. would need to be part of packet header + crc.
			};
			Engine::FlatHashMap<Engine::Net::IPv4Address, ConnInfo> ipToPlayer; // TODO: name

			std::vector<Engine::ECS::ComponentBitset> lastCompsBitsets;

			Engine::Net::IPv4Address address;	
			Engine::Net::Packet packet = {};
			const Engine::Net::IPv4Address group;
			Engine::Clock::TimePoint now = {};
			Engine::Clock::TimePoint lastUpdate = {};
			NeighborsComponent::Set lastNeighbors;

			// TODO: at some point we probably want to shrink this
			Engine::FlatHashMap<Engine::ECS::Entity, Engine::ECS::Entity> entToLocal;


		public:
			NetworkingSystem(SystemArg arg);
			void setup();
			void broadcastDiscover(); // TODO: better way to handle messages
			void run(float32 dt);
			int32 connectionsCount() const;
			int32 playerCount() const;
			void connectTo(const Engine::Net::IPv4Address& addr);
			void disconnect(Engine::ECS::Entity ent);

		private:
			struct AddConnRes {
				Engine::ECS::Entity ent;
				Connection& conn;
			};
			AddConnRes addConnection2(const Engine::Net::IPv4Address& addr);
			void addPlayer(const Engine::ECS::Entity ent);
			AddConnRes getOrCreateConnection(const Engine::Net::IPv4Address& addr);
			uint64 getConnectionKey(const Engine::Net::IPv4Address& addr);

			void dispatchMessage(Engine::ECS::Entity ent, Connection& from, const Engine::Net::MessageHeader* hdr);
			void updateNeighbors();
			void runServer();
			void runClient();

			template<MessageType::Type Type>
			void handleMessageType(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
				static_assert(Type != Type, "Unhandled network message type.");
			};
	};
}
