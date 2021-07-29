#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/ShapedString.hpp>


namespace Engine::Gui {
	class Button : public Panel {
		public:
			ShapedString label;

			virtual void render(Context& ctx) const override {
				Panel::render(ctx);
				ctx.drawString({0,0}, &label);
			}

			virtual bool canFocus() const override { return false; }
			//virtual bool canHover() const override { return false; }
	};
}
