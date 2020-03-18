#pragma once

// Engine
#include <Engine/Clock.hpp>
#include <Engine/Input/InputState.hpp>
#include <Engine/StaticRingBuffer.hpp>

// Game
#include <Game/System.hpp>

namespace Game {
	class InputSystem : public System {
		public:
			InputSystem(SystemArg arg);
			void tick(float32 dt);
			void queueInput(const Engine::Input::InputState& state);

		private:
			struct TimedInput {
				Engine::Input::InputState input;
				Engine::Clock::TimePoint time;
			};

			// TODO: is this large enough? what about things like mouse/axis inputs?
			Engine::StaticRingBuffer<TimedInput, 128> buffer;
	};
}
