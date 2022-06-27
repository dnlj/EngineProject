#pragma once

// Engine
#include <Engine/Gui/StringLine.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::UI {
	class Label : public StringLine {
		public:
			using StringLine::StringLine;

			virtual bool canHover() const override { return false; }
			virtual bool canFocus() const override { return false; }
	};
}
