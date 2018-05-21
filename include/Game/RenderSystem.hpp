#pragma once

// GLM
#include <glm/glm.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	extern glm::mat4 projection; // TODO: Make this not global
	extern glm::mat4 view; // TODO: Make this not global

	class RenderSystem : public SystemBase {
		public:
			RenderSystem(World& world);
			virtual void run(float dt) override;
	};
}
