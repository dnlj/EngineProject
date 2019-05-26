#pragma once

// Engine
#include <Engine/Input.hpp>

namespace Engine {
	struct InputState {
		Input input{};
		bool state = false; // TODO: will need to generalize this for more than just buttons (e.g. an axis)
	};
}
