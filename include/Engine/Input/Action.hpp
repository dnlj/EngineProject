#pragma once

// Engine
#include <Engine/ECS/Common.hpp>
#include <Engine/Input/ActionListener.hpp>


namespace Engine::Input {
	class Action {
		public:
			ActionId aid;
			Value state;
			ECS::Tick tick;

		public:
			Action(ActionId aid, Value state = {}, ECS::Tick tick = {})
				: aid{aid}
				, state{state}
				, tick{tick} {
			}
	};
}
