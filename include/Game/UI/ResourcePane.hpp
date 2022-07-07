#pragma once

// Engine
#include <Engine/UI/CollapsibleSection.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class ResourcePane : public EUI::CollapsibleSection {
		private:
			Panel* body;

		public:
			ResourcePane(EUI::Context* context);
			virtual void render() override;
	};
}
