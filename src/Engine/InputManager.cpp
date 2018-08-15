// GLFW
#include <GLFW/glfw3.h>

// Engine
#include <Engine/InputManager.hpp>

namespace Engine {
	InputManager::InputManager() {
		previousState.max_load_factor(0.5);
		currentState.max_load_factor(0.5);
	}
	
	bool InputManager::wasPressed(const std::string& name) const {
		const auto code = binds.find(name);
		if (code == binds.cend()) { return false; }

		const auto prev = previousState.find(code->second);
		if (prev != previousState.cend() && prev->second) { return false; }

		const auto curr = currentState.find(code->second);
		if (curr == currentState.cend() || !curr->second) { return false; }

		return true;
	}	
	
	bool InputManager::isPressed(const std::string& name) const {
		const auto code = binds.find(name);
		if (code == binds.cend()) { return false; }

		const auto curr = currentState.find(code->second);
		if (curr == currentState.cend() || !curr->second) { return false; }

		return true;
	}
	
	bool InputManager::wasReleased(const std::string& name) const {
		const auto code = binds.find(name);
		if (code == binds.cend()) { return false; }

		const auto prev = previousState.find(code->second);
		if (prev == previousState.cend() || !prev->second) { return false; }

		const auto curr = currentState.find(code->second);
		if (curr == currentState.cend() || curr->second) { return false; }

		return true;
	}

	void InputManager::bind(std::string name, ScanCode code) {
		binds[std::move(name)] = code;
	}

	glm::vec2 InputManager::getMousePosition() const {
		return mousePosition;
	}

	void InputManager::update() {
		previousState = currentState;
	}

	void InputManager::keyCallback(ScanCode code, int action) {
		if (action != GLFW_REPEAT) {
			currentState[code] = (action == GLFW_PRESS);
		}
	}

	void InputManager::mouseCallback(double x, double y) {
		mousePosition.x = static_cast<float>(x);
		mousePosition.y = static_cast<float>(y);
	}
}
