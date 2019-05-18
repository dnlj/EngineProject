#pragma once

// STD
#include <cstdint>
#include <array>


namespace Engine {
	// TODO: move
	enum class InputType : int8_t {
		KEYBOARD = 0,
		MOUSE = 1,
		GAMEPAD = 2,
	};

	struct Input {
		// TODO: add operators for sort
		InputType type;
		int code = -1;
	};

	// TODO: Move
	class InputSet : public std::array<Input, 4> {
	};
}
