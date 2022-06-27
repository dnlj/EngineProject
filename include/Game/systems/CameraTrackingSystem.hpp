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
			void update(float dt);
	};
}
