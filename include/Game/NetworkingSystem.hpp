#pragma once

// Engine
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/MessageStream.hpp>
#include <Engine/Net/Connection.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Engine.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	enum class MessageType : uint8 {
		UNKNOWN,
		DISCOVER_SERVER,
		SERVER_INFO,
		CONNECT,
		DISCONNECT,
		PING,
		ECS_COMP,
		ACTION,
	};

	// TODO: crtp specialization based on server/client
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
			Engine::Net::MessageStream reader;
			Engine::ECS::EntityFilter& connFilter;
			Engine::FlatHashMap<Engine::Net::IPv4Address, Engine::ECS::Entity> ipToPlayer;
			Engine::Net::Connection anyConn; // Used for unconnected messages


		public:
			NetworkingSystem(SystemArg arg);
			void setup();
			void broadcastDiscover(); // TODO: better way to handle messages
			void tick(float32 dt);
			int32 connectionsCount() const;
			void connectTo(const Engine::Net::IPv4Address& addr);

		private:
			void disconnect(const Engine::Net::Connection& conn);
			void addConnection(const Engine::Net::IPv4Address& addr);

			// TODO: rm
			void onDisconnect(const Engine::Net::Connection& conn);

			void dispatchMessage(Engine::Net::Connection& from);

			template<MessageType Type>
			void handleMessageType(Engine::Net::Connection& from) { static_assert(Type != Type, "Unhandled network message type."); };
	};
}
