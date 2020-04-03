#pragma once
// Engine
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/MessageStream.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class NetworkingSystem : public System {
		private:
			Engine::Net::UDPSocket socket;
			Engine::Net::MesssageStream stream;

		public:
			NetworkingSystem(SystemArg arg);
			void setup();
			void tick(float32 dt);
			void run(float32 dt);
	};
}
