#pragma once

// Engine
#include <Engine/Input/ActionListener.hpp>


namespace Engine::Input {
	class Action {
		public:
			ActionId aid;
			Value state;
			std::string name; // TODO: rm

		public:
			Action(ActionId aid, Value state = {}, std::string name = {})
				: aid{aid}
				, state{state}
				, name{std::move(name)} {
			}
			Action(const Action&) = delete;
			Action(Action&& other) = default;
	};
}
