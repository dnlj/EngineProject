#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/ShapedString.hpp>


namespace Engine::Gui {
	class Label : public Panel {
		protected:
			ShapedString str;

		public:
			ENGINE_INLINE void setText(std::string txt) {
				str = std::move(txt);
			}

			ENGINE_INLINE void setFont(Font font) {
				str.setFont(font);
			}

			ENGINE_INLINE void shape() {
				str.shape();
			}

			virtual void render(Context& ctx) const override {
				ctx.drawRect({0,0}, getSize(), {});
				ctx.drawString({0,0}, &str);
			}

			virtual bool canHover() const { return false; }
			virtual bool canFocus() const { return false; }
	};
}
