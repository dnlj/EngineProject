// Engine
#include <Engine/Input/InputManager.hpp>


namespace Engine::Input {
	void InputManager::update() {
		for (const auto& bind : binds) {
			if (bind.isActive()) {
				bind.hold();
			}
		}
	}

	void InputManager::processInput(const InputState& is) {
		const auto found = inputToMapping.find(is.input);
		if (found == inputToMapping.end()) { return; }

		for (const auto mapi : found->second) {
			auto& map = inputBindMappings[mapi];

			const bool preActive = map.isActive();
			map.processInput(is);
			const bool postActive = map.isActive();

			if (!preActive && postActive) {
				binds[map.getBindId()].press();
			} else if (preActive && !postActive) {
				binds[map.getBindId()].release();
			}
		}
	}

	BindId InputManager::createBind(std::string name) {
		binds.emplace_back(std::move(name));
		return static_cast<BindId>(binds.size()) - 1;
	}

	BindId InputManager::getBindId(const std::string& name) const {
		for (int i = 0; i < binds.size(); ++i) {
			if (binds[i].name == name) {
				return i;
			}
		}

		return -1;
	}

	Bind& InputManager::getBind(const std::string& name) {
		return binds[getBindId(name)];
	}

	Bind& InputManager::getBind(const BindId bid) {
		return binds[bid];
	}

	void InputManager::addInputBindMapping(const std::string& name, InputSequence inputs) {
		ENGINE_DEBUG_ASSERT(inputs.size() > 0, "InputSequence must have at least one input.");

		auto bid = getBindId(name);
		if (bid < 0) {
			bid = createBind(name);
		}

		inputBindMappings.emplace_back(bid, std::move(inputs));

		for (const auto& input : inputs) {
			if (input) {
				inputToMapping[input].push_back(
					static_cast<uint16_t>(inputBindMappings.size() - 1)
				);
			}
		}
	}
	glm::vec2 InputManager::getMousePosition() const {
		return mousePosition;
	}

	void InputManager::mouseCallback(int32 x, int32 y) {
		mousePosition.x = static_cast<float>(x);
		mousePosition.y = static_cast<float>(y);
	}
}
