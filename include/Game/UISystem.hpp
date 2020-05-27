#pragma once

// Engine
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/Input/ActionId.hpp>

// Game
#include <Game/System.hpp>

namespace Game {
	class UISystem : public System {
		public:
			UISystem(SystemArg arg);

			void setup();
			void run(float32 dt);

		private:
			void ui_connect();
			void ui_debug();
			void ui_network();

			std::array<Engine::Input::ActionId, 2> targetIds;
			Engine::ECS::EntityFilter& connFilter;
	};
}
