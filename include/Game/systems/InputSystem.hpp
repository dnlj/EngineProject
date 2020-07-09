#pragma once

// Engine
#include <Engine/Clock.hpp>
#include <Engine/Input/InputEvent.hpp>
#include <Engine/StaticRingBuffer.hpp>

// Game
#include <Game/System.hpp>

namespace Game {
	class InputSystem : public System {
		public:
			InputSystem(SystemArg arg);
			void tick(float32 dt);
			void queueInput(const Engine::Input::InputEvent& event);

		private:
			Engine::StaticRingBuffer<Engine::Input::InputEvent, 128> buffer;
	};
}
