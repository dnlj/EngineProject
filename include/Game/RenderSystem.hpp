#pragma once

// GLM
#include <glm/glm.hpp>

// Engine
#include <Engine/SystemBase.hpp>

namespace Game {
	extern glm::mat4 projection; // TODO: Make this not global
	extern glm::mat4 view; // TODO: Make this not global

	class RenderSystem : public Engine::SystemBase {
		public:
			RenderSystem();
			void run(float dt);
	};
}
