#pragma once

// Engine
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/MessageStream.hpp>
#include <Engine/Net/Connection.hpp>
#include <Engine/FlatHashMap.hpp>

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
	};

	class NetworkingSystem : public System {
		public:
			// TODO: find better way to handle this
			struct ServerInfo {
				Engine::Clock::TimePoint lastUpdate;
				std::string name;
				// TODO: name, player count, ping, private, tags, etc.
			};
			Engine::FlatHashMap<Engine::Net::IPv4Address, ServerInfo> servers;

		private:
			static constexpr auto timeout = std::chrono::milliseconds{10'000};
			Engine::Net::UDPSocket socket;
			Engine::Net::MessageStream reader;
			Engine::Net::MessageStream writer;
			Engine::FlatHashMap<Engine::Net::IPv4Address, Engine::Net::Connection> connections; // TODO: node map?

		public:
			NetworkingSystem(SystemArg arg);
			void setup();
			void broadcastDiscover(); // TODO: better way to handle messages
			void tick(float32 dt);
			int32 connectionsCount() const;
			void connect(const Engine::Net::IPv4Address& addr);
			void disconnect(const Engine::Net::IPv4Address& addr);

		private:
			void onConnect(const Engine::Net::Connection& conn);
			void onDisconnect(const Engine::Net::Connection& conn);
			Engine::Net::Connection& getConnection(const Engine::Net::IPv4Address& addr);

			void dispatchMessage(const Engine::Net::IPv4Address& from);

			template<MessageType Type>
			void handleMessageType(const Engine::Net::IPv4Address& from) { static_assert(Type != Type, "Unhandled network message type."); };
	};
}
