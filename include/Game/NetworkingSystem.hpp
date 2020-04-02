#pragma once
// Engine
#include <Engine/Net/UDPSocket.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class NetworkingSystem : public System {
		private:
			Engine::Net::UDPSocket socket;

		public:
			NetworkingSystem(SystemArg arg);
			void setup();
			void tick(float32 dt);
			void run(float32 dt);
	};
}
