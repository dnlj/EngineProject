#pragma once

// Engine
#include <Engine/Gui/Window.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class ConnectWindow : public EUI::Window {
		private:
			EUI::Panel* content = nullptr;

		public:
			ConnectWindow(EUI::Context* context);
	};
}
