#pragma once

// Engine
#include <Engine/Gui/CollapsibleSection.hpp>
#include <Engine/Gui/Slider.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class NetCondPane : public EUI::CollapsibleSection {
		public:
			NetCondPane(EUI::Context* context);
			EUI::Slider& addSlider(std::string_view txt);
	};
}
