#pragma once

// Engine
#include <Engine/Camera.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/ECS/EntityFilter.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class CameraTrackingSystem : public System {
		public:
			CameraTrackingSystem(SystemArg arg);
			void run(float dt);

		private:
			Engine::ECS::EntityFilter& activePlayerFilter;
	};
}
