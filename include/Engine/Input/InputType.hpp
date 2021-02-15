#pragma once

// STD
#include <cstdint>


namespace Engine::Input {
	enum class InputType : int8 {
		UNKNOWN = 0,
		KEYBOARD,
		MOUSE,
		MOUSE_WHEEL,
		GAMEPAD,

		KEYBOARD_AXIS = -KEYBOARD,
		MOUSE_AXIS = -MOUSE,
		GAMEPAD_AXIS = -GAMEPAD,
	};

	constexpr bool isAxisInput(InputType it) {
		return it < InputType::UNKNOWN;
	}
}
