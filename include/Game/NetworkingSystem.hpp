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
#include <Game/NeighborsComponent.hpp>


namespace Game {
	class NetworkingSystem : public System {
		public:
			// TODO: find better way to handle this
			struct ServerInfo {
				Engine::Clock::TimePoint lastUpdate;
				std::string name;
				// TODO: name, player count, ping, private, tags, etc.
			};

			#if ENGINE_CLIENT
				Engine::FlatHashMap<Engine::Net::IPv4Address, ServerInfo> servers;
			#endif

		private:
			static constexpr auto timeout = std::chrono::milliseconds{10'000};
			Engine::Net::UDPSocket socket;
			EntityFilter& connFilter;
			Engine::FlatHashMap<Engine::Net::IPv4Address, Engine::ECS::Entity> ipToPlayer;
			std::vector<Engine::ECS::ComponentBitset> lastCompsBitsets;

			Engine::Net::IPv4Address address;	
			Engine::Net::Packet packet = {};
			Engine::Net::Connection anyConn; // Used for unconnected messages
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
			void connectTo(const Engine::Net::IPv4Address& addr);
			void disconnect(Engine::ECS::Entity ent);

		private:
			struct AddConnRes {
				Engine::ECS::Entity ent;
				Engine::Net::Connection& conn;
			};
			AddConnRes addConnection(const Engine::Net::IPv4Address& addr);

			void dispatchMessage(Engine::ECS::Entity ent, Engine::Net::Connection& from);
			void updateNeighbors();
			void runServer();
			void runClient();

			template<MessageType::Type Type>
			void handleMessageType(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
				static_assert(Type != Type, "Unhandled network message type.");
			};
	};
}
