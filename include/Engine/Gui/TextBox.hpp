#pragma once

// Engine
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class TextBox : public StringLine {
		private:
			uint32 caret = 0;
			float32 caretX = 0;
			glm::vec2 pad = {5,5}; // TODO: probably pull from font size / theme

		public:
			using StringLine::StringLine;

			ENGINE_INLINE void autoSize() {
				StringLine::autoSize();
				StringLine::offset(pad);
				setSize(getSize() + pad + pad);
			}

			virtual void render(Context& ctx) const override {
				glm::vec2 pos = {0,0};
				const glm::vec2 size = getSize();
				const glm::vec4 bg = {0.3,0.3,0.3,1};
				const glm::vec4 bo = {0,0,0,1};

				ctx.drawRect(pos, size, bg);

				ctx.drawRect(pos, {size.x, 1}, bo);
				ctx.drawRect(pos, {1, size.y}, bo);
				ctx.drawRect(pos + glm::vec2{0, size.y - 1}, {size.x, 1}, bo);
				ctx.drawRect(pos + glm::vec2{size.x - 1, 0}, {1, size.y}, bo);

				pos += pad;

				ctx.drawString(getStringOffset(), &getShapedString());

				if (ctx.getFocus() == this && ctx.isBlinking()) {
					const auto& str = getShapedString();
					//const auto ssize = str.getBounds().getSize();

					ctx.drawRect(
						//pos + glm::vec2{ssize.x, 0},
						pos + glm::vec2{caretX, 0},
						{1, str.getFont()->getLineHeight()},
						bo
					);
				}
			}

			virtual void onAction(Action act) override {
				switch (act) {
					case Action::MoveCharLeft: { moveCharLeft(); break; }
					case Action::MoveCharRight: { moveCharRight(); break; }
				}
			}

			virtual void onBeginFocus() override {
				ctx->registerCharCallback(this, [this](wchar_t ch) {
					// Filter non-printable characters
					if (ch < ' ' || ch > '~') { return true; }
					// TODO: insert at caret
					setText(getText() + static_cast<char>(ch));
					return true;
				});
			};

			virtual void onEndFocus() override {
				ctx->deregisterCharCallback(this);
			};

		private:
			void moveCharLeft() {
				if (caret > 0) {
					--caret;
					updateCaretPos();
				}
			}

			void moveCharRight() {
				++caret;
				updateCaretPos();
			}

			void updateCaretPos() {
				const auto& glyphs = getShapedString().getGlyphShapeData();
				caretX = 0;
				uint32 last = 0;
				uint32 i = 0;

				for (const auto& glyph : glyphs) {
					if (glyph.cluster != last) {
						++i;
						last = glyph.cluster;
					}

					if (i == caret) { break; }

					caretX += glyph.advance.x;
				}

				if (i < caret) {
					caret = i;
				} else {
					ctx->updateBlinkTime();
				}
			}
	};
}
