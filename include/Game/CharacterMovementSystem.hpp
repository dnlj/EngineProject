#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	class CharacterMovementSystem : public SystemBase {
		public:
			CharacterMovementSystem(World& world);
			void run(float dt);
	};
}
