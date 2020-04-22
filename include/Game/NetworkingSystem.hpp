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
		CONNECT,
		DISCONNECT,
		PING,
		ECS_COMP,
	};

	class NetworkingSystem : public System {
		private:
			static constexpr auto timeout = std::chrono::milliseconds{10'000};
			Engine::Net::UDPSocket socket;
			Engine::Net::MessageStream reader;
			Engine::Net::MessageStream writer;
			Engine::FlatHashMap<Engine::Net::IPv4Address, Engine::Net::Connection> connections; // TODO: node map?
			Engine::Net::IPv4Address addr;

		public:
			NetworkingSystem(SystemArg arg);
			void setup();
			void tick(float32 dt);
			int32 connectionsCount() const;
			void connect(const Engine::Net::IPv4Address& addr);
			void disconnect(const Engine::Net::IPv4Address& addr);

		private:
			void onConnect(const Engine::Net::Connection& conn);
			void onDisconnect(const Engine::Net::Connection& conn);
			Engine::Net::Connection& getConnection(const Engine::Net::IPv4Address& addr);
			void ping(const Engine::Net::IPv4Address& addr);

			void dispatchMessage();

			template<MessageType Type>
			void handleMessageType() { static_assert(Type != Type, "Unhandled network message type."); };
	};
}
