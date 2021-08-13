#pragma once

// Engine
#include <Engine/Input/InputId.hpp>

namespace Engine::Input {
	class InputState {
		public:
			InputId id{};
			union {
				int32 i32 = 0;
				float32 f32;
			} value;
	};

	// This is assumed in a few places. Usually for copying the value of the anonymous union.
	static_assert(sizeof(InputState) == sizeof(InputState::id) + sizeof(InputState::value));
}
