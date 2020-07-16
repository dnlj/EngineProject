#pragma once

// Game
#include <Game/systems/ActionSystem.hpp>

// TODO: get rid of this file. We dont need a file for one function

namespace Game {
	template<class Listener>
	void ActionSystem::addListener(Engine::Input::ActionId aid, Listener&& listener) {
		ENGINE_DEBUG_ASSERT(aid < count(), "Attempting to add listener to invalid action");
		actionIdToListeners[aid].emplace_back(std::forward<Listener>(listener));
	}
}
