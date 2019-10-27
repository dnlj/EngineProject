#pragma once

// STD
#include <string>

// Engine
#include <Engine/Input/InputManager.hpp>

namespace Game {
	class  InputComponent {
		public:
			Engine::Input::InputManager* inputManager = nullptr;
	};
}
