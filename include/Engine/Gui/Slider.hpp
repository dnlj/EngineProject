#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Label.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	class Slider : public Panel {
		private:
			float64 min = 0;
			float64 max = 1;
			float64 p = 0.5;
			Label* label = nullptr;

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

			void setLimits(float64 min, float64 max) {
				this->min = min;
				this->max = max;
				updateLabel();
			}

			ENGINE_INLINE auto getValue() const noexcept { return std::lerp(min, max, p); }

			ENGINE_INLINE void updateLabel() {
				label->autoText(fmt::format("{:.3}", getValue()));
			}

			virtual void render() const override {
				const auto sz = getSize();
				constexpr float32 hw = 8; // handle width

				ctx->drawRect({}, sz, {1,1,0,0.5});
				ctx->drawRect({sz.x * p - (hw * 0.5f), 0}, {hw, sz.y}, {1,0,1,0.5});
			}

			virtual void preLayout() {
				label->setRelPos((getSize() - label->getSize()) * 0.5f);
			}

			virtual void onBeginActivate() override {
				const auto func = [this](const glm::vec2 pos) {
					p = (ctx->getCursor().x - getPos().x) / getWidth();
					p = glm::clamp(p, 0.0, 1.0);
					updateLabel();
				};
				func(ctx->getCursor());
				ctx->registerMouseMove(this, std::move(func));
			}

			virtual void onEndActivate() override {
				ctx->deregisterMouseMove(this);
			}
	};
}
