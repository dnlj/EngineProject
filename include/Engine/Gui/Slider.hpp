#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	class Slider : public Panel {
		private:
			float32 p = 0.5f;

		public:
			// TODO: min, max
			// TODO: int types, float types
			// TODO: snap intervals
			// TODO: vertical option
			Slider(Context* context) : Panel{context} {
				// TODO: button or just manual draw? probably manual. no need for button
				setSize({32, 16});
			}

			ENGINE_INLINE auto getValue() const noexcept { return p; }

			virtual void render() const override {
				const auto pos = glm::vec2{};
				const auto sz = getSize();
				constexpr float32 hw = 8; // handle width

				ctx->drawRect(pos, sz, {1,1,0,0.5});
				ctx->drawRect({pos.x + sz.x * p - (hw * 0.5f), pos.y}, {hw, sz.y}, {1,0,1,0.5});
			}

			virtual void onBeginActivate() override {
				const auto func = [this](const glm::vec2 pos) {
					p = (ctx->getCursor().x - getPos().x) / getWidth();
					p = glm::clamp(p, 0.0f, 1.0f);
				};
				func(ctx->getCursor());
				ctx->registerMouseMove(this, std::move(func));
			}

			virtual void onEndActivate() override {
				ctx->deregisterMouseMove(this);
			}
	};
}
