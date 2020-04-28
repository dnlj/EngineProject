#pragma once

// Engine
#include <Engine/Input/InputManager.hpp>


namespace Engine::Input {
	template<class Listener>
	auto InputManager::addBind(const InputSequence& inputs, Listener&& listener) -> BindId {
		auto bid = static_cast<BindId>(binds.size());
		binds.emplace_back(inputs, std::forward<Listener>(listener));

		for (auto& input : inputs) {
			if (input) {
				bindLookup[input].push_back(bid);
			}
		}

		return bid;
	}
}
