#pragma once

// Engine
#include <Engine/UI/Window.hpp>
#include <Engine/UI/TextBox.hpp>

// Game
#include <Game/UI/ui.hpp>

namespace Game::UI {
	class MapPreviewDragArea;
	class MapPreview : public EUI::Window {
		private:
			EUI::TextBox* xMove = nullptr;
			EUI::TextBox* yMove = nullptr;
			EUI::TextBox* xZoom = nullptr;
			EUI::TextBox* yZoom = nullptr;
			MapPreviewDragArea* area = nullptr;

		public:
			MapPreview(EUI::Context* context);
	};
}
