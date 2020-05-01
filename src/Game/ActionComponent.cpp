// Game
#include <Game/ActionComponent.hpp>


namespace Game {
	void ActionComponent::grow(Engine::Input::ActionId size) {
		// TODO: fix. this is dumb.
		while (actions.size() != size) {
			actions.emplace_back(static_cast<Engine::Input::ActionId>(actions.size()));
		}
	}

	Engine::Input::Action& ActionComponent::get(Engine::Input::ActionId aid) {
		return actions[aid];
	};

	Engine::Input::Value ActionComponent::getValue(Engine::Input::ActionId aid) {
		return get(aid).state;
	};
}
