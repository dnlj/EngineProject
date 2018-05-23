// GLFW
#include <GLFW/glfw3.h>

// Engine
#include <Engine/InputManager.hpp>

namespace Engine {
	InputManager::InputManager() {
		previousState.max_load_factor(0.5);
		currentState.max_load_factor(0.5);
	}
	
	bool InputManager::wasPressed(const std::string& name) {
		const auto& code = binds[name];
		return !previousState[code] && currentState[code];
	}	
	
	bool InputManager::isPressed(const std::string& name) {
		const auto& code = binds[name];
		return currentState[code];
	}
	
	bool InputManager::wasReleased(const std::string& name) {
		const auto& code = binds[name];
		return previousState[code] && !currentState[code];
	}

	void InputManager::bind(std::string name, ScanCode code) {
		binds[std::move(name)] = code;
	}

	void InputManager::update() {
		previousState = currentState;
	}

	void InputManager::callback(ScanCode code, int action) {
		if (action != GLFW_REPEAT) {
			currentState[code] = (action == GLFW_PRESS);
		}
	}
}
