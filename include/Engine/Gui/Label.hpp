#pragma once

// Engine
#include <Engine/Gui/StringLine.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	class Label : public StringLine {
		public:
			using StringLine::StringLine;

			virtual void render() const override {
				ctx->drawRect({0,0}, getSize(), {1,0,0,0});
				ctx->drawString(getStringOffset(), &getShapedString());
			}

			virtual bool canHover() const override { return false; }
			virtual bool canFocus() const override { return false; }
	};
}
