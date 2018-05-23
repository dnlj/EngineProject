#pragma once

// STD
#include <string>

// Engine
#include <Engine/InputManager.hpp>

namespace Game {
	class  InputComponent {
		public:
			Engine::InputManager* inputManager = nullptr;
	};
}
