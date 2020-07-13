#pragma once

// Engine
#include <Engine/Input/ActionListener.hpp>


namespace Engine::Input {
	class Action {
		public:
			ActionId aid;
			Value state;

		public:
			Action(ActionId aid, Value state = {})
				: aid{aid}
				, state{state} {
			}
			Action(const Action&) = delete;
			Action(Action&& other) = default;
	};
}
