#pragma once

// Engine
#include <Engine/UI/Window.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class ConsoleWindow final : public EUI::Window {
		public:
			ConsoleWindow(EUI::Context* context);
	};
}
