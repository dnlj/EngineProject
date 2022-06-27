#pragma once

// Engine
#include <Engine/UI/Bindable.hpp>
#include <Engine/UI/Label.hpp>
#include <Engine/UI/Context.hpp>


namespace Engine::UI {
	class Slider : public Panel, public Bindable<Slider> {
		private:
			float64 min = 0;
			float64 max = 1;
			float64 p = 0.5;
			ShapedString str;
			glm::vec2 strOff = {};

		public:
			// TODO: label string or decimal count
			// TODO: snap intervals
			// TODO: vertical option
			Slider(Context* context) : Panel{context} {
				setSize({32, 16});
				str.setFont(ctx->getTheme().fonts.body);
				updateLabel();
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
				setBindableValue();
				return *this;
			}

			ENGINE_INLINE Slider& setValue(float64 v) {
				return setPercentage((v - min) / (max - min));
			}

			ENGINE_INLINE float64 getValue() const noexcept { return std::lerp(min, max, p); }

			ENGINE_INLINE void updateLabel() {
				str = fmt::format("{:.3}", getValue());
				str.shape();
				strOff.y = str.getFont()->getBodyHeight();
			}

			virtual void render() override {
				const auto sz = getSize();
				constexpr float32 hw = 8; // handle width
				const auto& theme = ctx->getTheme();
				ctx->drawRect({}, sz, theme.colors.feature);
				ctx->drawRect({sz.x * p - (hw * 0.5f), 0}, {hw, sz.y}, theme.colors.button);
				ctx->drawString(strOff, &str, theme.colors.foregroundAlt);
			}

			virtual void postLayout() override {
				strOff.x = (getWidth() - str.getBounds().getWidth()) * 0.5f;
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
