#pragma once

// Engine
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/Connection.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Engine.hpp>

// Game
#include <Game/System.hpp>
#include <Game/MessageType.hpp>


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
			Engine::Net::Connection reader; // TODO: is this needed anymore? just use anyConn? or do we need anyConnRead, anyConnWrite
			Engine::ECS::EntityFilter& connFilter;
			Engine::FlatHashMap<Engine::Net::IPv4Address, Engine::ECS::Entity> ipToPlayer;
			Engine::Net::Connection anyConn; // Used for unconnected messages
			const Engine::Net::IPv4Address group;


		public:
			NetworkingSystem(SystemArg arg);
			void setup();
			void broadcastDiscover(); // TODO: better way to handle messages
			void tick(float32 dt);
			int32 connectionsCount() const;
			void connectTo(const Engine::Net::IPv4Address& addr);

		private:
			void disconnect(const Engine::Net::Connection& conn);
			Engine::Net::Connection& addConnection(const Engine::Net::IPv4Address& addr);

			// TODO: rm
			void onDisconnect(const Engine::Net::Connection& conn);

			void dispatchMessage(Engine::ECS::Entity ent, Engine::Net::Connection& from);

			template<MessageType::Type Type>
			void handleMessageType(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity ent) {
				static_assert(Type != Type, "Unhandled network message type.");
			};
	};
}
