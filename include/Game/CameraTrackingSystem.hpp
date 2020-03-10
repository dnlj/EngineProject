#pragma once

// Engine
#include <Engine/Camera.hpp>
#include <Engine/ECS/Entity.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class CameraTrackingSystem : public System {
		public:
			CameraTrackingSystem(SystemArg arg);
			void setup(Engine::Camera& camera);
			void run(float dt);

			Engine::ECS::Entity focus;

		private:
			Engine::Camera* camera;
	};
}
