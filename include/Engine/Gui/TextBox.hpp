#pragma once

// Engine
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class TextBox : public StringLine {
		private:
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

				if (ctx.getFocus() == this && ctx.isBlinking()) {
					const auto& str = getShapedString();
					const auto ssize = str.getBounds().getSize();

					ctx.drawRect(
						pos + glm::vec2{ssize.x, 0},
						{1, str.getFont()->getLineHeight()},
						bo
					);
				}

				ctx.drawString(getStringOffset(), &getShapedString());
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
	};
}
