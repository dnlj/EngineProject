#pragma once

// Engine
#include <Engine/UI/TextFeed.hpp>
#include <Engine/UI/Window.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class ConsolePanel final : public EUI::PanelT {
		private:
			EUI::TextFeed* feed;
			EUI::TextBox* input;

		public:
			ConsolePanel(EUI::Context* context);
			virtual bool onAction(EUI::ActionEvent act) override;

			// TODO: up/down command history
	};

	class ConsoleWindow final : public EUI::Window {
		public:
			ConsoleWindow(EUI::Context* context);
	};
}
