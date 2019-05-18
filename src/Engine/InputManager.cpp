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

	void InputManager::bindkey(ScanCode code, const std::string& name) {
		auto bid = getBindId(name);

		if (scanCodeToBindID.size() <= code) {
			scanCodeToBindID.resize(code + 1, -1);
		}

		scanCodeToBindID[code] = bid;
	}

	void InputManager::bindMouseButton(MouseButton button, const std::string& name) {
		auto bid = getBindId(name);

		if (mouseButtonToBindId.size() <= button) {
			mouseButtonToBindId.resize(button + 1, -1);
		}

		mouseButtonToBindId[button] = bid;
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

	BindId InputManager::getBindId(const std::string& name) {
		const bool inserted = bindToBindID.find(name) == bindToBindID.end();
		auto& bid = bindToBindID[name];

		if (inserted) {
			bid = static_cast<BindId>(bindToBindID.size() - 1);

			currentState.resize(bid + 1);
			previousState.resize(bid + 1);
		}

		return bid;
	}

	void InputManager::keyCallback(ScanCode code, int action) {
		if (action != GLFW_REPEAT && code < scanCodeToBindID.size()) {
			auto bid = scanCodeToBindID[code];

			if (bid >= 0) {
				currentState[bid] = (action == GLFW_PRESS);
				insertBindEvent({bid, getBindStateFromInt(action)});
			}
		}
	}

	void InputManager::mouseCallback(double x, double y) {
		mousePosition.x = static_cast<float>(x);
		mousePosition.y = static_cast<float>(y);
	}

	void InputManager::mouseCallback(MouseButton button, int action) {
		if (button < mouseButtonToBindId.size()) {
			auto bid = mouseButtonToBindId[button];

			if (bid >= 0) {
				currentState[bid] = (action == GLFW_PRESS);
				insertBindEvent({bid, getBindStateFromInt(action)});
			}
		}
	}

	void InputManager::insertBindEvent(BindEvent event) {
		bindEventQueue.events[bindEventQueue.size] = std::move(event);
		++bindEventQueue.size;
	}
}
