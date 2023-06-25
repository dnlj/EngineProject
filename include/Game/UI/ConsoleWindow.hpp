#pragma once

// Engine
#include <Engine/UI/Window.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Engine::UI {
	class ConsolePanel;
}

namespace Game::UI {
	class ConsoleWindow final : public EUI::Window {
		public:
			ConsoleWindow(EUI::Context* context);
			EUI::ConsolePanel* get() const noexcept;
			void submit(std::string_view text);
	};
}
