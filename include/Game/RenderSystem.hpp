#pragma once

// GLM
#include <glm/glm.hpp>

// Engine
#include <Engine/ECS/Common.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class RenderSystem : public SystemBase {
		public:
			RenderSystem(World& world);
			virtual void run(float dt) override;

			Engine::ECS::EntityID focus;

		private:
			glm::mat4 projection;
			glm::mat4 view;
	};
}
