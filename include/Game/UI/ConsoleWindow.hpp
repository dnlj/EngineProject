#pragma once

// Engine
#include <Engine/UI/TextFeed.hpp>
#include <Engine/UI/Window.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {

	class ConsoleWindow : public EUI::Window {
		private:
			EUI::TextFeed* feed;

		public:
			ConsoleWindow(EUI::Context* context);
	};
}
