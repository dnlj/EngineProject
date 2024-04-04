#pragma once

// Engine
#include <Engine/UI/Window.hpp>
#include <Engine/UI/TextBox.hpp>

// Game
#include <Game/UI/ui.hpp>

namespace Game::UI {
	class ZonePreview : public EUI::Window {
		public:
			ZonePreview(EUI::Context* context);
	};
}
