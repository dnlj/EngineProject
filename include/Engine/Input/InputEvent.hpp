#pragma once

// Engine
#include <Engine/Clock.hpp>
#include <Engine/Input/InputState.hpp>


namespace Engine::Input {
	/**
	 * An input at a time.
	 */
	class InputEvent {
		public:
			InputState state;
			Clock::TimePoint time;
	};
}
