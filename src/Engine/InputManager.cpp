// STD
#include <algorithm>

// GLFW
#include <GLFW/glfw3.h>

// Engine
#include <Engine/InputManager.hpp>

// Helpers
namespace {
	constexpr Engine::BindState getBindStateFromInt(int state) {
		switch (state) {
			case GLFW_RELEASE: return Engine::BindState::RELEASE;
			case GLFW_PRESS: return Engine::BindState::PRESS;
			default: return Engine::BindState::INVALID;
		}
	}
}

namespace Engine {
	InputManager::InputManager() {
		bindToBindID.max_load_factor(0.5f);
	}
	
	bool InputManager::wasPressed(const std::string& name) const {
		const auto bp = bindToBindID.find(name);
		if (bp == bindToBindID.cend()) { return false; }
		return wasPressed(bp->second);
	}

	bool InputManager::wasPressed(BindId bid) const {
		if (bid > currentState.size()) { return false; }
		return currentState[bid] && !previousState[bid];
	}
	
	bool InputManager::isPressed(const std::string& name) const {
		const auto bp = bindToBindID.find(name);
		if (bp == bindToBindID.cend()) { return false; }
		return isPressed(bp->second);
	}

	bool InputManager::isPressed(BindId bid) const {
		if (bid > currentState.size()) { return false; }
		return currentState[bid];
	}
	
	bool InputManager::wasReleased(const std::string& name) const {
		const auto bp = bindToBindID.find(name);
		if (bp == bindToBindID.cend()) { return false; }
		return wasReleased(bp->second);
	}

	bool InputManager::wasReleased(BindId bid) const {
		if (bid > currentState.size()) { return false; }
		return previousState[bid] && !currentState[bid];
	}

	void InputManager::bind(ScanCode code, const std::string& name) {
		const bool inserted = bindToBindID.find(name) == bindToBindID.end();
		auto& bid = bindToBindID[name];

		if (inserted) {
			bid = nextBindID;
			++nextBindID;

			currentState.resize(bid + 1);
			previousState.resize(bid + 1);
		}

		if (scanCodeToBindID.size() <= code) {
			scanCodeToBindID.resize(code + 1, -1);
		}

		scanCodeToBindID[code] = bid;
	}

	glm::vec2 InputManager::getMousePosition() const {
		return mousePosition;
	}

	void InputManager::update() {
		previousState = currentState;
		bindEventQueue.size = 0;
	}

	const BindEventQueue& InputManager::getBindEventQueue() const {
		return bindEventQueue;
	};

	void InputManager::keyCallback(ScanCode code, int action) {
		if (action != GLFW_REPEAT && code < scanCodeToBindID.size()) {
			auto bid = scanCodeToBindID[code];

			if (bid >= 0) {
				currentState[bid] = (action == GLFW_PRESS);

				bindEventQueue.events[bindEventQueue.size] = {bid, getBindStateFromInt(action)};
				++bindEventQueue.size;
			}
		}
	}

	void InputManager::mouseCallback(double x, double y) {
		mousePosition.x = static_cast<float>(x);
		mousePosition.y = static_cast<float>(y);
	}
}
