// Game
#include <Game/systems/InputSystem.hpp>
#include <Game/World.hpp>


namespace Game {
	InputSystem::InputSystem(SystemArg arg) : System{arg} {
	}

	void InputSystem::tick() {
		// TODO: only if not performing rollback. Maybe add a way for systems to opt out instead of if-ing every system
		if (world.isPerformingRollback()) { return; }

		const auto curTime = world.getTickTime();
		const auto nextTime = curTime + world.getTickInterval();

		while (!buffer.empty()) {
			auto& ie = buffer.front();

			if (ie.time < nextTime) {
				commands[ie.command](ie.value);
				buffer.pop();
			} else {
				break;
			}
		}
	}
}
