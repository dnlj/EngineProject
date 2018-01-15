#pragma once

// Engine
#include <Engine/SystemBase.hpp>

namespace Game {
	class CharacterMovementSystem : public Engine::SystemBase {
		public:
			CharacterMovementSystem();
			void run(float dt);
	};
}