#pragma once

// Game
#include <Game/System.hpp>


namespace Game {
	class PhysicsInterpSystem : public System {
		public:
			PhysicsInterpSystem(SystemArg arg) : System{arg} {}
			void update(float32 dt);
	};
}
