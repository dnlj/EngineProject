#pragma once

// Game
#include <Game/System.hpp>


namespace Game {
	class CharacterMovementSystem : public System {
		public:
			CharacterMovementSystem(SystemArg arg);
			void tick();
	};
}
