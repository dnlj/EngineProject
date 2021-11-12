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

			void setAction(Callback func) { action = func; }

			virtual void render() const override {
				ctx->drawRect({0,0}, getSize(), {1,0,0,0.2});
				ctx->drawString(getStringOffset(), &getShapedString());
			}

			virtual void onEndActivate() override {
				if (action && getBounds().contains(ctx->getCursor())) {
					action(this);
				}
			}
	};
}
