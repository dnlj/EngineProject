#pragma once

// STD
#include <functional>

// Engine
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class Button : public StringLine {
		private:
			using Callback = std::function<void()>;
			Callback beginCallback;
			Callback endCallback;

		public:
			using StringLine::StringLine;

			void setBeginActive(Callback func) { beginCallback = func; }
			void setEndActive(Callback func) { endCallback = func; }

			virtual void render() const override {
				ctx->drawRect({0,0}, getSize(), {1,0,0,0.2});
				ctx->drawString(getStringOffset(), &getShapedString());
			}

			virtual void onBeginActivate() override {
				if (beginCallback) { beginCallback(); }
			}

			virtual void onEndActivate() override {
				if (endCallback) { endCallback(); }
			}
	};
}
