#pragma once

// Engine
#include <Engine/UI/StringLine.hpp>
#include <Engine/UI/Context.hpp>


namespace Engine::UI {
	class Label : public StringLine {
		public:
			using StringLine::StringLine;

			virtual bool canHover() const override { return false; }
			virtual bool canFocus() const override { return false; }
	};
}
