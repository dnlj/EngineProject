#pragma once

// Engine
#include <Engine/Input/InputId.hpp>
#include <Engine/Input/Value.hpp>

namespace Engine::Input {
	class InputState {
		public:
			InputId id{};
			Value value;
	};
}
