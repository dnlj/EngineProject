// Engine
#include <Engine/Input/InputManager.hpp>


namespace Engine::Input {
	void InputManager::update() {
		for (const auto& bind : buttonBinds) {
			if (bind.isActive()) {
				bind.hold();
			}
		}
	}

	void InputManager::processInput(const InputState& is) {
		if (is.id.isAxis()) {
			processAxisInput(is);
		} else {
			processButtonInput(is);
		}
	}

	void InputManager::processAxisInput(const InputState& is) {
		const auto found = axisMappings.find(is.id);
		if (found == axisMappings.end()) { return; }

		for (const auto aid : found->second) {
			axisBinds[aid].value = is.valuef;
			// TODO: listeners
		}
	}

	void InputManager::processButtonInput(const InputState& is) {
		const auto found = buttonToMapping.find(is.id);
		if (found == buttonToMapping.end()) { return; }

		for (const auto mapi : found->second) {
			auto& map = buttonMappings[mapi];

			const bool preActive = map.isActive();
			map.processInput(is);
			const bool postActive = map.isActive();

			if (!preActive && postActive) {
				buttonBinds[map.getBindId()].press();
			} else if (preActive && !postActive) {
				buttonBinds[map.getBindId()].release();
			}
		}
	}

	BindId InputManager::createButtonBind(std::string name) {
		buttonBinds.emplace_back(std::move(name));
		return static_cast<BindId>(buttonBinds.size()) - 1;
	}

	BindId InputManager::getButtonBindId(const std::string& name) const {
		for (int i = 0; i < buttonBinds.size(); ++i) {
			if (buttonBinds[i].name == name) {
				return i;
			}
		}

		return -1;
	}

	Bind& InputManager::getButtonBind(const std::string& name) {
		return buttonBinds[getButtonBindId(name)];
	}

	Bind& InputManager::getButtonBind(const BindId bid) {
		return buttonBinds[bid];
	}

	void InputManager::addButtonMapping(const std::string& name, InputSequence inputs) {
		ENGINE_DEBUG_ASSERT(inputs.size() > 0, "InputSequence must have at least one input.");

		auto bid = getButtonBindId(name);
		if (bid < 0) {
			bid = createButtonBind(name);
		}

		buttonMappings.emplace_back(bid, std::move(inputs));

		for (const auto& input : inputs) {
			if (input) {
				buttonToMapping[input].push_back(
					static_cast<uint16_t>(buttonMappings.size() - 1)
				);
			}
		}
	}

	AxisId InputManager::createAxisBind(std::string_view name) {
		axisBinds.emplace_back(std::string{name});
		return static_cast<AxisId>(axisBinds.size()) - 1;
	}

	AxisId InputManager::getAxisId(std::string_view name) const {
		for (AxisId aid = 0; aid < axisBinds.size(); ++aid) {
			if (axisBinds[aid].name == name) {
				return aid;
			}
		}
		return -1;
	}

	AxisBind& InputManager::getAxisBind(const AxisId aid) {
		return axisBinds[aid];
	}

	AxisBind& InputManager::getAxisBind(std::string_view name) {
		return getAxisBind(getAxisId(name));
	}

	void InputManager::addAxisMapping(std::string_view name, InputId axis) {
		auto aid = getAxisId(name);
		if (aid < 0) {
			aid = createAxisBind(name);
		}

		axisMappings[axis].push_back(aid);
	}

	float32 InputManager::getAxisValue(AxisId aid) {
		return getAxisBind(aid).value;
	}
}
