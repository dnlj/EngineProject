#pragma once
// Engine
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/MessageStream.hpp>
#include <Engine/Net/Connection.hpp>
#include <Engine/FlatHashMap.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class NetworkingSystem : public System {
		private:
			Engine::Net::UDPSocket socket;
			Engine::Net::MessageStream msg;
			Engine::FlatHashMap<Engine::Net::IPv4Address, uint8> ipToConnection;
			std::vector<Engine::Net::Connection> connections;

		public:
			NetworkingSystem(SystemArg arg);
			void setup();
			void tick(float32 dt);

		private:
			Engine::Net::Connection& getConnection(const Engine::Net::IPv4Address& addr);

			template<int32>
			void handleMessage(const Engine::Net::IPv4Address& from);
	};
}
