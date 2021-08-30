#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/ShapedString.hpp>


namespace Engine::Gui {
	class StringLine : public Panel {
		private:
			ShapedString str;
			glm::vec2 strOff = {};

		public:
			using Panel::Panel;


			ENGINE_INLINE void setText(std::string txt) {
				str = std::move(txt);
				if (str.getFont()) { shape(); }
			}

			ENGINE_INLINE const auto& getText() const noexcept { return str.getString(); }

			ENGINE_INLINE void setFont(Font font) {
				str.setFont(font);
				postLayout();
			}


			ENGINE_INLINE void shape() { str.shape(); }

			ENGINE_INLINE void autoSize() {
				const auto& bounds = str.getBounds();
				const auto sz = bounds.getRoundSize();
				setSize({bounds.max.x, str.getFont()->getLineHeight()});
			}

			ENGINE_INLINE void offset(glm::vec2 off) { strOff += off; }

			virtual void render(Context& ctx) const override = 0;

			virtual void postLayout() override {
				if (const auto font = str.getFont(); font) {
					// By default (0,0) is the text baseline so we offset to make it top left.
					// To get the true bounds we would need to check `str.getBounds()` but that
					// would give us an inconsistent baseline.
					//
					// See "Glyph Metrics" illustrations at: https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html#section-3
					const auto offsetToTopLeft = font->getAscent();

					// Baseline offset + center in panel
					strOff.y = offsetToTopLeft + 0.5f * (getHeight() - font->getLineHeight());
				}
			}

		protected:
			ENGINE_INLINE const ShapedString& getShapedString() const { return str; }
			ENGINE_INLINE auto getStringOffset() const { return strOff; }
	};
}