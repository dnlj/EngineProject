#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/ShapedString.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	class Label : public Panel {
		protected:
			ShapedString str;
			glm::vec2 offset = {};

		public:
			ENGINE_INLINE void setText(std::string txt) {
				str = std::move(txt);
				if (str.getFont()) { shape(); }

				// By default we (0,0) is the text baseline so we offset to make it top left.
				// To get the true bounds we would need to check `str.getBounds()` but that
				// would give us an inconsistent baseline.
				//
				// See "Glyph Metrics" illustrations at: https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html#section-3
				offset.y = str.getFont()->getAscent();
			}

			ENGINE_INLINE void setFont(Font font) {
				str.setFont(font);
			}

			ENGINE_INLINE void shape() {
				str.shape();
			}

			ENGINE_INLINE void autoSize() {
				const auto& bounds = str.getBounds();
				const auto sz = bounds.getRoundSize();
				setSize({bounds.max.x, str.getFont()->getLineHeight()});
			}

			virtual void render(Context& ctx) const override {
				ctx.drawRect({0,0}, getSize(), {1,0,0,1});
				ctx.drawString(offset, &str);
			}

			virtual bool canHover() const { return false; }
			virtual bool canFocus() const { return false; }
	};
}
