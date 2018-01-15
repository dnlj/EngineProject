#pragma once

// Engine
#include <Engine/SystemBase.hpp>

namespace Game {
	class PhysicsSystem : public Engine::SystemBase {
		public:
			void run(float dt);
	};
}