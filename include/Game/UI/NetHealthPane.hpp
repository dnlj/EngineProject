#pragma once

// Engine
#include <Engine/UI/CollapsibleSection.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class NetHealthPane : public EUI::CollapsibleSection {
		public:
			NetHealthPane(EUI::Context* context);
	};
}
