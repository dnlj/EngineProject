#pragma once

// GLM
#include <glm/glm.hpp>

// Engine
#include <Engine/ECS/Common.hpp>
#include <Engine/Camera.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class RenderSystem : public SystemBase {
		public:
			RenderSystem(World& world);
			void setup(Engine::Camera& camera);
			virtual void run(float dt) override;

			Engine::ECS::EntityID focus;

		private:
			Engine::Camera* camera;
	};
}
