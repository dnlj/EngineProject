#pragma once

// Engine
#include <Engine/Gui/CollapsibleSection.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class NetHealthPane : public EUI::CollapsibleSection {
		public:
			NetHealthPane(EUI::Context* context);
	};
}
