#pragma once

// Engine
#include <Engine/SystemBase.hpp>

namespace Game {
	class RenderableTestMovement : public Engine::SystemBase {
		public:
			RenderableTestMovement();
			void run(float dt);
	};
}