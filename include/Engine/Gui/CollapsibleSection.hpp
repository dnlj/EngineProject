#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>
#include <Engine/Gui/DirectionalLayout.hpp>
#include <Engine/Gui/Button.hpp>


namespace Engine::Gui {
	class CollapsibleSection : public Panel {
		private:
			Button* btn = nullptr;
			Panel* content = nullptr;
			bool open = true;
			float32 height;

			struct Layout : DirectionalLayout {
				using DirectionalLayout::DirectionalLayout;

				virtual float32 getAutoHeight(const Panel* panel) const override {
					auto section = reinterpret_cast<const CollapsibleSection*>(panel);
					auto content = section->getContent();
					const auto h = content->isEnabled() ? content->getAutoHeight() + gap : 0;
					return section->btn->getHeight() + h;
				}
			};

		public:
			CollapsibleSection(Context* context) : Panel{context} {
				setLayout(new Layout{Direction::Vertical, Align::Stretch, Align::Stretch});
				height = getHeight();

				btn = ctx->constructPanel<Button>();
				btn->setText("Section Test");
				btn->setAction([&]{ toggle(); });
				btn->setMinSize({0, 32});
				btn->setMaxSize({INFINITY, 32});
				btn->setHeight(32);

				content = ctx->constructPanel<Panel>();
				content->setMinSize({32,32});
				addChildren({btn, content});
			}

			ENGINE_INLINE void setTitle(std::string title) {
				btn->setText(std::move(title));
			}

			ENGINE_INLINE void toggle() {
				if (open) { height = getHeight(); }
				open = !open;

				content->setEnabled(open);
				setHeight(open ? height : btn->getHeight());
			}

			ENGINE_INLINE Panel* getContent() const noexcept { return content; }

			virtual void render() const override {
				ctx->drawRect({0,0}, getSize(), {0,0.5,0.5,1.0});
			}
	};
}
