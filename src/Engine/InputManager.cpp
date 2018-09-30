// GLFW
#include <GLFW/glfw3.h>

// Engine
#include <Engine/InputManager.hpp>


namespace Engine {
	InputManager::InputManager() {
		bindToBindID.max_load_factor(0.5f);
	}
	
	bool InputManager::wasPressed(const std::string& name) const {
		const auto bp = bindToBindID.find(name);
		if (bp == bindToBindID.cend()) { return false; }
		return wasPressed(bp->second);
	}

	bool InputManager::wasPressed(BindID bid) const {
		if (bid > currentState.size()) { return false; }
		return currentState[bid] && !previousState[bid];
	}
	
	bool InputManager::isPressed(const std::string& name) const {
		const auto bp = bindToBindID.find(name);
		if (bp == bindToBindID.cend()) { return false; }
		return isPressed(bp->second);
	}

	bool InputManager::isPressed(BindID bid) const {
		if (bid > currentState.size()) { return false; }
		return currentState[bid];
	}
	
	bool InputManager::wasReleased(const std::string& name) const {
		const auto bp = bindToBindID.find(name);
		if (bp == bindToBindID.cend()) { return false; }
		return wasReleased(bp->second);
	}

	bool InputManager::wasReleased(BindID bid) const {
		if (bid > currentState.size()) { return false; }
		return previousState[bid] && !currentState[bid];
	}

	void InputManager::bind(ScanCode code, const std::string& name) {
		auto& bid = bindToBindID[name];

		if (bid == 0) {
			bid = nextBindID;
			++nextBindID;

			currentState.resize(bid + 1);
			previousState.resize(bid + 1);
		}

		if (scanCodeToBindID.size() <= code) {
			scanCodeToBindID.resize(code + 1);
		}

		scanCodeToBindID[code] = bid;
	}

	glm::vec2 InputManager::getMousePosition() const {
		return mousePosition;
	}

	void InputManager::update() {
		previousState = currentState;
	}

	void InputManager::keyCallback(ScanCode code, int action) {
		if (action != GLFW_REPEAT) {
			auto bid = scanCodeToBindID[code];

			if (bid < currentState.size()) {
				currentState[bid] = (action == GLFW_PRESS);
			}
		}
	}

	void InputManager::mouseCallback(double x, double y) {
		mousePosition.x = static_cast<float>(x);
		mousePosition.y = static_cast<float>(y);
	}
}
