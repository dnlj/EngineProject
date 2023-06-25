// Engine
#include <Engine/UI/ConsolePanel.hpp>
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/TextBox.hpp>
#include <Engine/Unicode/UTF8.hpp>

// Game
#include <Game/UI/ConsoleWindow.hpp>


namespace Game::UI {
	ConsoleWindow::ConsoleWindow(EUI::Context* context) : Window{context} {
		setTitle("Console");
		setSize({650, 500});
		setRelPos({512,64+300+8});
		setContent(ctx->constructPanel<EUI::ConsolePanel>());
	}

	EUI::ConsolePanel* ConsoleWindow::get() const noexcept {
		return static_cast<EUI::ConsolePanel*>(getContent());
	}

	void ConsoleWindow::push(std::string_view text) {
		auto* panel = static_cast<EUI::ConsolePanel*>(getContent());
		panel->push(text);
	}
}
