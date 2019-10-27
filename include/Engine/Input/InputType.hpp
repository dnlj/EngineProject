#pragma once

// STD
#include <cstdint>


namespace Engine::Input {
	enum class InputType : int8_t {
		UNKNOWN = 0,
		KEYBOARD = 1,
		MOUSE = 2,
		GAMEPAD = 3,
	};
}
