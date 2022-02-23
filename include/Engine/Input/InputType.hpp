#pragma once

// STD
#include <cstdint>


namespace Engine::Input {
	enum class InputType : int8 {
		Unknown = 0,
		Keyboard,
		Mouse,
		MouseWheel,
		Gamepad,

		KeyboardAxis = -Keyboard,
		MouseAxis = -Mouse,
		GamepadAxis = -Gamepad,
	};

	constexpr inline bool isAxisInput(InputType it) noexcept {
		return it < InputType::Unknown;
	}
}
