// Game
#include <Game/systems/InputSystem.hpp>
#include <Game/World.hpp>


namespace Game {
	InputSystem::InputSystem(SystemArg arg) : System{arg} {
	}

	void InputSystem::tick() {
		// TODO: only if not performing rollback. Maybe add a way for systems to opt out instead of if-ing every system
		const auto curTime = world.getTickTime();
		const auto nextTime = curTime + world.getTickInterval();

		while (!buffer.empty()) {
			auto& ie = buffer.front();

			if (ie.time < nextTime) {
				engine.inputManager.processInput(ie.state);
				buffer.pop();
			} else {
				break;
			}
		}
	}

	void InputSystem::queueInput(const Engine::Input::InputEvent& event) {
		ENGINE_DEBUG_ASSERT(!buffer.full(), "Too many inputs");
		buffer.emplace(event);
	}
}
