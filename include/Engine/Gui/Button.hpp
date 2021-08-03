#pragma once

// Engine
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class Button : public Label {
		public:
			virtual void render(Context& ctx) const override {
				ctx.drawRect({0,0}, getSize(), {1,0,0,0.2});
				ctx.drawString({0,0}, &str);
			}

			virtual bool canHover() const { return true; }
			virtual bool canFocus() const { return true; }
	};
}
