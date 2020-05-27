#pragma once

// Engine
#include <Engine/ECS/EntityFilter.hpp>

// Game
#include <Game/System.hpp>

namespace Game {
	class UISystem : public System {
		public:
			UISystem(SystemArg arg);
			void run(float32 dt);

		private:
			void ui_connect();
			void ui_network();
			Engine::ECS::EntityFilter& connFilter;
	};
}
