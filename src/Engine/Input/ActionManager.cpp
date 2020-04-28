// Engine
#include <Engine/Input/ActionManager.hpp>


namespace Engine::Input {
	Action::Action(Value state, std::string name)
		: state{state}
		, name{std::move(name)} {
	}

	ActionId ActionManager::create(const std::string& name) {
		auto found = actionNameToId.find(name);
		if (found == actionNameToId.end()) {
			found = actionNameToId.emplace(name, static_cast<ActionId>(actions.size())).first;
			actions.emplace_back(Value{}, name);
		} else {
			ENGINE_WARN("Attempting to create action that already exists: ", name);
		}

		return found->second;
	}

	ActionId ActionManager::getId(const std::string& name) {
		auto found = actionNameToId.find(name);
		if (found == actionNameToId.end()) {
			ENGINE_WARN("Attempting to get the id of a nonexistent action: ", name);
			return create(name);
		}

		return found->second;
	}

	Action& ActionManager::get(const std::string& name) {
		return get(getId(name));
	}

	Action& ActionManager::get(ActionId aid) {
		return actions[aid];
	}
	
	Value ActionManager::getValue(const std::string& name) {
		return get(getId(name)).state;
	}

	Value ActionManager::getValue(ActionId aid) {
		return get(aid).state;
	}

	void ActionManager::addListener(ActionId aid, ActionListener listener) {
		actions[aid].listeners.push_back(std::move(listener));
	}
}
