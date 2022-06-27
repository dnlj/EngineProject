#pragma once

// Engine
#include <Engine/Gui/CollapsibleSection.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class NetGraphPane : public EUI::CollapsibleSection {
		public:
			NetGraphPane(EUI::Context* context);
	};
}
