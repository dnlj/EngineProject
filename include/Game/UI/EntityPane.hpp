#pragma once

// Engine
#include <Engine/UI/CollapsibleSection.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class EntityPane : public EUI::CollapsibleSection {
		public:
			EntityPane(EUI::Context* context);
	};
}
