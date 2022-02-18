#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/ShapedString.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	class StringLine : public Panel {
		private:
			ShapedString str;
			glm::vec2 off = {};
			glm::vec2 pad = {};

		public:
			StringLine(Context* context) : Panel{context} {
				auto& theme = ctx->getTheme();
				setFont(theme.fonts.body);
				//pad.x = theme.sizes.pad1;
				pad.x = 0;
				pad.y = pad.x;
			}

			ENGINE_INLINE void setPadding(glm::vec2 p) noexcept { pad = p; }
			ENGINE_INLINE void setPadding(float32 p) noexcept { setPadding({p, p}); }
			ENGINE_INLINE auto getPadding() const noexcept { return pad; }

			ENGINE_INLINE void insertText(uint32 i, std::string_view text) {
				auto& s = str.getStringMutable();
				s.insert(s.begin() + i, text.begin(), text.end());
				if (str.getFont()) { shape(); }
			}

			ENGINE_INLINE void setText(std::string_view txt) {
				str = txt;
				if (str.getFont()) { shape(); }
			}

			ENGINE_INLINE const auto& getText() const noexcept { return str.getString(); }

			ENGINE_INLINE void setFont(Font font) {
				str.setFont(font);
				postLayout();
			}

			ENGINE_INLINE Font getFont() const {
				return str.getFont();
			}

			ENGINE_INLINE void shape() { str.shape(); }

			ENGINE_INLINE glm::vec2 getStringSize() {
				const auto& bounds = str.getBounds();
				const auto sz = bounds.getRoundSize();
				return {sz.x, str.getFont()->getLineHeight()};
			}

			ENGINE_INLINE void autoSize() {
				setSize(getStringSize() + pad + pad);
			}

			/**
			 * Sets the text and resizes the panel to fit.
			 */
			ENGINE_INLINE void autoText(std::string_view txt) {
				setText(txt);
				autoSize();
			}

			virtual void render() override {
				ctx->drawRect({0,0}, getSize(), {1,0,0,0});
				//if (const auto font = str.getFont(); font) { ctx->drawRect({off.x, off.y - font->getAscent()}, {getStringSize().x, font->getBodyHeight()}, {1,0,0,1});}
				//ctx->drawRect(getStringOffset(), {getWidth(), 1}, {1,0,0,1});
				ctx->drawString(getStringOffset(), &getShapedString());
			}

			virtual void postLayout() override {
				if (const auto font = str.getFont(); font) {
					// By default (0,0) is the text baseline so we offset to make it top left.
					// To get the true bounds we would need to check `str.getBounds()` but that
					// would give us an inconsistent baseline.
					//
					// See "Glyph Metrics" illustrations at: https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html#section-3
					off.y = font->getAscent();

					// Baseline offset + center in panel
					off.x = pad.x;
					off.y += std::max(0.5f * (getHeight() - font->getBodyHeight()), pad.y);
				}
			}

		protected:
			ENGINE_INLINE auto& getTextMutable() noexcept { return str.getStringMutable(); }
			ENGINE_INLINE const ShapedString& getShapedString() const { return str; }
			ENGINE_INLINE glm::vec2 getStringOffset() const { return off; }
	};
}
