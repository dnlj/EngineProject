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

		public:
			CollapsibleSection(Context* context) : Panel{context} {
				setLayout(new DirectionalLayout{Direction::Vertical, Align::Start, Align::Start});
				height = getHeight();

				btn = ctx->constructPanel<Button>();
				btn->setText("Section Test");
				btn->setAction([&]{ toggle(); });

				content = ctx->constructPanel<Panel>();
				addChildren({btn, content});
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
				btn->setSize({getSize().x, 32});

				auto* layout = reinterpret_cast<DirectionalLayout*>(getLayout());
				content->setSize({getSize().x, getSize().y - btn->getSize().y - layout->getGap()});
			}
	};
}
