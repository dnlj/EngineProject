#pragma once

// STD
#include <string>

// Engine
#include <Engine/InputManager2.hpp>

namespace Game {
	class  InputComponent {
		public:
			Engine::InputManager2* inputManager = nullptr;
	};
}
