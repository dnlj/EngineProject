#pragma once

// STD
#include <cstdint>


namespace Engine::Input {
	enum class InputType : int8 {
		UNKNOWN = 0,

		KEYBOARD,
		MOUSE,
		GAMEPAD,

		MOUSE_AXIS,
		GAMEPAD_AXIS,

		_COUNT,
	};
}
