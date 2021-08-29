#pragma once

// Engine
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class TextBox : public StringLine {
		public:
			using StringLine::StringLine;
			
			virtual void render(Context& ctx) const override {
				ctx.drawRect({0,0}, getSize(), {1,0,0,0.2});
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
