#pragma once

// STD
#include <functional>

// Engine
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class Button : public StringLine {
		private:
			using Callback = std::function<void(Button*)>;
			Callback action;

		public:
			using StringLine::StringLine;
			Button(Context* context) : StringLine{context} {
				setPadding(ctx->getTheme().sizes.pad1);
			}

			void setAction(Callback func) { action = func; }

			virtual void render() override {
				ctx->drawRect({0,0}, getSize(), ctx->getTheme().colors.button);
				StringLine::render();
			}

			virtual void onEndActivate() override {
				if (action && getBounds().contains(ctx->getCursor())) {
					action(this);
				}
			}
	};
}
