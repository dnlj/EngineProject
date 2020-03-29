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
			auto& ie = buffer.back();

			if (ie.time < nextTime) {
				engine.inputManager.processInput(ie.state);
				buffer.pop();
			} else {
				break;
			}
		}

		engine.inputManager.update();
	}

	void InputSystem::queueInput(const Engine::Input::InputEvent& event) {
		ENGINE_DEBUG_ASSERT(!buffer.full(), "Too many inputs");
		buffer.emplace(event);
	}
}
