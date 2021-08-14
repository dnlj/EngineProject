#pragma once

// Engine
#include <Engine/Gui/DirectionalLayout.hpp>
#include <Engine/Gui/Label.hpp>


namespace Engine::Gui {
	class Window : public Panel {
		private:
			class Title : public Label {
				public:
					Window* win;

				public:
					Title(Context* context, Window* window) : Label{context}, win{window} {}
					
					virtual void onBeginActivate() override {
						win->tracking = true;
						win->offset = win->getContext()->getCursor() - win->getPos();
					}

					virtual void onEndActivate() override {
						win->tracking = false;
					}

					virtual bool canHover() const override { return true; }
					virtual bool canFocus() const override { return true; }
			};

			bool tracking = false;
			glm::vec2 offset = {};
			Title* title;

		public:
			Window(Context* context) : Panel{context} {
				setLayout(new DirectionalLayout{Direction::Vertical, Align::Stretch});

				title = ctx->createPanel<Title>(this);
				addChild(title);
				title->setFont(ctx->font_b);
				title->setText("Window Title");
				title->setRelPos({0, 0});
				title->autoSize();

				ctx->registerMouseMove(this, [this](glm::vec2 cursor) {
					if (tracking) {
						const auto bounds = getParent()->getBounds();
						const auto max = bounds.max - title->getSize();
						auto p = glm::clamp(cursor - offset, bounds.min, max);
						setPos(p);
					}
				});

				// TODO: resize
			}
	};
}
