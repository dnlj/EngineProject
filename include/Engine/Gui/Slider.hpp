#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Label.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	class Slider : public Panel {
		public:
			using Callback = std::function<void(float64)>;
			using GetFunc = std::function<void(Slider&)>;
			using SetFunc = std::function<void(Slider&)>;

		private:
			float64 min = 0;
			float64 max = 1;
			float64 p = 0.5;
			Label* label = nullptr;
			SetFunc setFunc;

		public:
			// TODO: label string or decimal count
			// TODO: snap intervals
			// TODO: vertical option
			Slider(Context* context) : Panel{context} {
				label = ctx->constructPanel<Label>();
				addChild(label);
				setSize({32, 16});
				updateLabel();
			}

			Slider& bind(GetFunc get, SetFunc set) {
				setFunc = {};
				get(*this);
				setFunc = std::move(set);
				ctx->addPanelUpdateFunc(this, [this, g=std::move(get)](Panel*){ g(*this); });
				return *this;
			}

			Slider& setLimits(float64 min, float64 max) {
				this->min = min;
				this->max = max;
				updateLabel();
				return *this;
			}

			ENGINE_INLINE Slider& setPercentage(float64 p) {
				p = std::clamp(p, 0.0, 1.0);
				if (p == this->p) { return *this; }
				this->p = p;
				updateLabel();
				if (setFunc) { setFunc(*this); }
				return *this;
			}

			ENGINE_INLINE Slider& setValue(float64 v) {
				return setPercentage((v - min) / (max - min));
			}

			ENGINE_INLINE float64 getValue() const noexcept { return std::lerp(min, max, p); }

			ENGINE_INLINE void updateLabel() {
				label->autoText(fmt::format("{:.3}", getValue()));
			}

			virtual void render() override {
				const auto sz = getSize();
				constexpr float32 hw = 8; // handle width
				const auto& theme = ctx->getTheme();
				ctx->drawRect({}, sz, theme.colors.feature);
				ctx->drawRect({sz.x * p - (hw * 0.5f), 0}, {hw, sz.y}, theme.colors.button);
			}

			virtual void preLayout() {
				label->setRelPos((getSize() - label->getSize()) * 0.5f);
			}

			virtual bool onBeginActivate() override {
				const auto func = [this](const glm::vec2 pos) {
					float32 v = (ctx->getCursor().x - getPos().x) / getWidth();
					setPercentage(v);
				};
				func(ctx->getCursor());
				ctx->registerMouseMove(this, std::move(func));
				return true;
			}

			virtual void onEndActivate() override {
				ctx->deregisterMouseMove(this);
			}
	};
}
