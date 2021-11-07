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

				//virtual void autoHeight(Panel* panel) override {
				//	auto section = reinterpret_cast<CollapsibleSection*>(panel);
				//	section->getContent()->autoHeight();
				//	DirectionalLayout::autoHeight(panel);
				//}
			};

		public:
			CollapsibleSection(Context* context) : Panel{context} {
				setLayout(new DirectionalLayout{Direction::Vertical, Align::Stretch, Align::Stretch});
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

				auto TODO_rm = ctx->createPanel<Panel>(this);
				TODO_rm->setMaxSize({62, 62});
				TODO_rm->setSize({8,8});
			}

			ENGINE_INLINE void setTitle(std::string title) {
				btn->setText(std::move(title));
			}

			ENGINE_INLINE void toggle() {
				open = !open;

				content->setEnabled(open);
				setHeight(open ? height : btn->getHeight());
			}

			ENGINE_INLINE auto getContent() const noexcept { return content; }

			virtual void render() const override {
				ctx->drawRect({0,0}, getSize(), {0,0.5,0.5,1.0});
			}

			virtual void preLayout() override {
				if (open) { height = getHeight(); }

				//auto* layout = reinterpret_cast<DirectionalLayout*>(getLayout());
				//content->setSize({getSize().x, getSize().y - btn->getSize().y - layout->getGap()});
			}
	};
}
