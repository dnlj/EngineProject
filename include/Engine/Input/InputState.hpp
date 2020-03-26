#pragma once

// Engine
#include <Engine/Input/InputId.hpp>

namespace Engine::Input {
	struct InputState {
		InputId id{};
		union {
			int32 valuei = 0;
			float32 valuef;
			bool active;
		};
	};
}
