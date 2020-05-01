// Game
#include <Game/ActionSystem.hpp>
#include <Game/World.hpp>
#include <Game/ActionComponent.hpp>


namespace Game {
	ActionSystem::ActionSystem(SystemArg arg)
		: System{arg}
		, actionFilter{world.getFilterFor<ActionComponent>()} {
	}

	void ActionSystem::processAction(const Engine::Input::ActionEvent& event) {
		ENGINE_DEBUG_ASSERT(event.aid < count(), "Attempting to process invalid action");
		auto& actionComp = world.getComponent<ActionComponent>(event.ent);
		auto& action = actionComp.get(event.aid);
		auto& listeners = actionIdToListeners[event.aid];
		for (auto& l : listeners) {
			l(event.ent, event.aid, event.val, action.state);
		}
		action.state = event.val;
	}

	Engine::Input::ActionId ActionSystem::create(const std::string& name) {
		auto found = actionNameToId.find(name);
		if (found == actionNameToId.end()) {
			const auto next = actionNameToId.size();
			found = actionNameToId.emplace(name, static_cast<Engine::Input::ActionId>(next)).first;
			actionIdToListeners.emplace_back();

			for (auto& ent : actionFilter) {
				auto& actC = world.getComponent<ActionComponent>(ent);
				actC.grow(static_cast<Engine::Input::ActionId>(next + 1));
			}
		} else {
			ENGINE_WARN("Attempting to create action that already exists: ", name);
		}

		return found->second;
	}

	Engine::Input::ActionId ActionSystem::getId(const std::string& name) {
		auto found = actionNameToId.find(name);
		if (found == actionNameToId.end()) {
			ENGINE_WARN("Attempting to get the id of a nonexistent action: ", name);
			return create(name);
		}

		return found->second;
	}

	
	Engine::Input::ActionId ActionSystem::count() const {
		return static_cast<Engine::Input::ActionId>(actionNameToId.size());
	}
}
