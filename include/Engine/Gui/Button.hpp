#pragma once

// STD
#include <functional>

// Engine
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class Button : public Label {
		private:
			using Callback = std::function<void()>;
			Callback beginCallback;
			Callback endCallback;

		public:
			using Label::Label;

			void setBeginActive(Callback func) { beginCallback = func; }
			void setEndActive(Callback func) { endCallback = func; }

			virtual void render(Context& ctx) const override {
				ctx.drawRect({0,0}, getSize(), {1,0,0,0.2});
				Label::render(ctx);
			}

			virtual bool canHover() const override { return true; }
			virtual bool canFocus() const override { return true; }

			virtual void onBeginActivate() override {
				if (beginCallback) { beginCallback(); }
			}

			virtual void onEndActivate() override {
				if (endCallback) { endCallback(); }
			}

	};
}
