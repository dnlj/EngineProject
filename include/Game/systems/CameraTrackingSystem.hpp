#pragma once

// Engine
#include <Engine/Camera.hpp>
#include <Engine/ECS/Entity.hpp>

// Game
#include <Game/System.hpp>
#include <Game/EntityFilter.hpp>


namespace Game {
	class CameraTrackingSystem : public System {
		public:
			CameraTrackingSystem(SystemArg arg);
			void run(float dt);

		private:
			EntityFilter& activePlayerFilter;
	};
}
