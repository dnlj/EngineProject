#pragma once

// Engine
#include <Engine/Input/InputId.hpp>

namespace Engine::Input {
	struct InputState {
		InputId input{};
		bool state = false; // TODO: will need to generalize this for more than just buttons (e.g. an axis)
	};
}
