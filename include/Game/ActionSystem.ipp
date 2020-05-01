#pragma once

// Game
#include <Game/ActionSystem.hpp>


namespace Game {
	template<class... String>
	auto ActionSystem::getId(const std::string& name1, const std::string& name2, String&&... nameN) {
		return std::array<Engine::Input::ActionId, 2 + sizeof...(String)>{
			getId(name1), getId(name2), getId(std::forward<String>(nameN)) ...
		};
	}

	template<class Listener>
	void ActionSystem::addListener(Engine::Input::ActionId aid, Listener&& listener) {
		ENGINE_DEBUG_ASSERT(aid < count(), "Attempting to add listener to invalid action");
		actionIdToListeners[aid].emplace_back(std::forward<Listener>(listener));
	}
}
