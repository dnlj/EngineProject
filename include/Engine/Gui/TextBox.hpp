#pragma once

// Engine
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class TextBox : public StringLine {
		private:
			int64 caret = 0;
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
					case Action::MoveCharLeft: {
						// TODO: we need to be able to go -1 so we can co before first char
						if (caret > -1) {
							--caret;
							updateCaretPos();
							ctx->updateBlinkTime();
						}
						break;
					}
					case Action::MoveCharRight: {
						if (caret < static_cast<int64>(getText().size())) {
							++caret;
							updateCaretPos();
							ctx->updateBlinkTime();
						}
						break;
					}
				}
			}

			virtual void onBeginFocus() override {
				ctx->registerCharCallback(this, [this](wchar_t ch) {
					// Filter non-printable characters
					if (ch < ' ' || ch > '~') { return true; }
					setText(getText() + static_cast<char>(ch));
					return true;
				});
			};

			virtual void onEndFocus() override {
				ctx->deregisterCharCallback(this);
			};

		private:
			void updateCaretPos() {
				caretX = 0;
				const auto& glyphs = getShapedString().getGlyphShapeData();
				for (const auto& glyph : glyphs) {
					if (glyph.cluster > caret) { break; }
					caretX += glyph.advance.x;
				}
			}
	};
}
