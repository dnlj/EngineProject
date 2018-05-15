#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	class PhysicsSystem : public SystemBase {
		public:
			PhysicsSystem(World& world);
			void run(float dt);
	};
}
