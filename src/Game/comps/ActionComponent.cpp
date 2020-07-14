// Game
#include <Game/comps/ActionComponent.hpp>


namespace Game {
	void ActionComponent::grow(Engine::Input::ActionId size) {
		// TODO: fix. this is dumb.
		const auto g = [&](auto& v){
			while (v.size() != size) {
				v.emplace_back(static_cast<Engine::Input::ActionId>(v.size()));
			}
		};

		g(actions);
		for (auto& v : actionHistory) { g(v); }
	}

	Engine::Input::Action& ActionComponent::get(Engine::Input::ActionId aid) {
		return actions[aid];
	};

	Engine::Input::Value ActionComponent::getValue(Engine::Input::ActionId aid) {
		return get(aid).state;
	};
}
