#pragma once

// STD
#include <functional>

// Engine
#include <Engine/UI/Label.hpp>


namespace Engine::UI {
	class Button : public StringLine {
		public:
			using Callback = std::function<void(Button*)>;

		private:
			Callback action;

		public:
			using StringLine::StringLine;
			Button(Context* context) : StringLine{context} {
				setPadding(ctx->getTheme().sizes.pad1);
			}

			void setAction(Callback func) { action = func; }

			virtual void render() override {
				ctx->setColor(ctx->getTheme().colors.button);
				ctx->drawRect({0,0}, getSize());
				StringLine::render();
			}

			virtual void onEndActivate() override {
				if (action && getBounds().contains(ctx->getCursor())) {
					action(this);
				}
			}
	};
}
