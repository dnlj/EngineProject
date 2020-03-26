#pragma once

// Engine
#include <Engine/Input/InputId.hpp>

namespace Engine::Input {
	class InputState {
		public:
			InputId id{};
			union {
				int32 value = 0;
				float32 valuef;
			};
	};

	// This is assumed in a few places. Usually for copying the value of the anonymous union.
	static_assert(sizeof(InputState) == sizeof(InputState::id) + sizeof(InputState::value));
}
