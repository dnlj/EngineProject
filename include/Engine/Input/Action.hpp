#pragma once

// Engine
#include <Engine/Input/ActionListener.hpp>


namespace Engine::Input {
	class Action {
		public:
			Value state;
			std::string name;
			std::vector<ActionListener> listeners;

		public:
			Action(Value state, std::string name);
			Action(const Action&) = delete;
			Action(Action&& other) = default;

			void set(Value value) {
				for (auto& l : listeners) {
					l(value, state);
				}
				state = value;
			}
	};
}
