#pragma once

// Engine
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class TextBox : public Label {
		public:
			using Label::Label;

			virtual bool canHover() const override { return true; }
			virtual bool canFocus() const override { return true; }

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
