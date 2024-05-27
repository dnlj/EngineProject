#pragma once

// Engine
#include <Engine/UI/Window.hpp>

// Game
#include <Game/UI/ui.hpp>

namespace Game::UI {
	class TerrainPreview : public EUI::Window {
		public:
			TerrainPreview(EUI::Context* context);
	};
}
