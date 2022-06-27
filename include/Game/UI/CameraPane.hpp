#pragma once

// Engine
#include <Engine/UI/CollapsibleSection.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class CameraPane : public EUI::CollapsibleSection {
		public:
			CameraPane(EUI::Context* context);
	};
}
