#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	class CameraTrackingSystem : public SystemBase {
		public:
			CameraTrackingSystem(World& world);
			void setup(Engine::Camera& camera);
			virtual void run(float dt) override;

			Engine::ECS::Entity focus;

		private:
			Engine::Camera* camera;
	};
}
