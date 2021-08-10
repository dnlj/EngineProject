#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>
#include <Engine/Gui/DirectionalLayout.hpp>
#include <Engine/Gui/Button.hpp>


namespace Engine::Gui {
	class CollapsibleSection : public Panel {
		private:
			DirectionalLayout lay;
			Button* btn = nullptr;
			Panel* content = nullptr;
			bool open = true;
			float32 height = 128;

		public:
			CollapsibleSection(Context* ctx)
				: lay{Direction::Vertical, Align::Start} {

				btn = ctx->createPanel<Button>();
				addChild(btn);
				btn->setText("Section Test");
				btn->setFont(ctx->font_b); // TODO:
				btn->shape();
				btn->setBeginActive([&]{
					ENGINE_LOG("Set Begin Active");
					toggle();
				});
				btn->setEndActive([&]{
					ENGINE_LOG("Set End Active");
				});

				content = ctx->createPanel<Panel>();
				addChild(content);
			}

			ENGINE_INLINE void toggle() {
				open = !open;

				content->setEnabled(open);
				setHeight(open ? height : btn->getHeight());
			}

			virtual void render(Context& ctx) const override {
				ctx.drawRect({0,0}, getSize(), {0,0.5,0.5,1.0});
			}

			virtual void layout() override {
				btn->setSize({getSize().x, 32});
				content->setSize({getSize().x, getSize().y - btn->getSize().y - lay.getGap()});
				lay.layout(this);
			}
	};
}
