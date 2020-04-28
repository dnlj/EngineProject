// Engine
#include <Engine/Input/InputManager.hpp>


namespace Engine::Input {
	void InputManager::processInput(const InputState& is) {
		const auto found = bindLookup.find(is.id);
		if (found == bindLookup.end()) { return; }
		
		for (const auto bid : found->second) {
			auto& bind = binds[static_cast<std::underlying_type_t<decltype(bid)>>(bid)];
			
			const auto pre = bind.getState();
			bind.processInput(is);
			const auto post = bind.getState();
			
			if (pre.value != post.value) {
				bind.notify(post, pre);
			}
		}
	}

	/*
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
	}*/
}
