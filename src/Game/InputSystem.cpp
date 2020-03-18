// Game
#include <Game/InputSystem.hpp>
#include <Game/World.hpp>


namespace Game {
	InputSystem::InputSystem(SystemArg arg) : System{arg} {
	}

	void InputSystem::tick(float32 dt) {
		const auto curTime = world.getTickTime();
		const auto nextTime = curTime + world.getTickInterval();

		while (!buffer.empty()) {
			auto& ti = buffer.back();

			if (ti.time < nextTime) {
				engine.inputManager.processInput(ti.input);
				buffer.pop();
			} else {
				break;
			}
		}
	}

	void InputSystem::queueInput(const Engine::Input::InputState& state) {
		ENGINE_ASSERT(!buffer.full(), "Too many inputs");
		buffer.emplace(state, Engine::Clock::now());
	}
}
