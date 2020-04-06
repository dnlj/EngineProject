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
		PING,
		TEST,
	};

	class NetworkingSystem : public System {
		private:
			Engine::Net::UDPSocket socket;
			Engine::Net::MessageStream msg;
			Engine::FlatHashMap<Engine::Net::IPv4Address, uint8> ipToConnection;
			std::vector<Engine::Net::Connection> connections;
			Engine::Net::IPv4Address addr;

		public:
			NetworkingSystem(SystemArg arg);
			void setup();
			void tick(float32 dt);

		private:
			Engine::Net::Connection& getConnection(const Engine::Net::IPv4Address& addr);

			void dispatchMessage();

			template<MessageType Type>
			void handleMessageType() { static_assert(Type != Type, "Unhandled network message type."); };
	};
}
