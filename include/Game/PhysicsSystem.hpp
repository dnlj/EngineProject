#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	class PhysicsSystem : public SystemBase {
		public:
			PhysicsSystem(World& world);
			virtual void run(float dt) override;
	};
}
