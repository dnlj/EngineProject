#pragma once

// Engine
#include <Engine/UI/Button.hpp>
#include <Engine/UI/DirectionalLayout.hpp>


namespace Engine::UI {
	class DropdownOption : public StringLine {
		public:
			bool selected = false;
			uint64 id;

		public:
			using StringLine::StringLine;
			
			virtual void render() override {
				const auto& theme = ctx->getTheme();
				const auto color = selected
					? theme.colors.backgroundSelection
					: ctx->getHover() == this
						? theme.colors.backgroundAlt
						: theme.colors.background;
				ctx->setColor(color);
				ctx->drawRect({0,0}, getSize());
				StringLine::render();
			}
			
			virtual bool onBeginActivate();
	};

	class DropdownContent : public Panel {
		public:
			class Dropdown* dropdown;

		private:
			DropdownOption* selected = nullptr;

		public:
			using Panel::Panel;
			virtual void onEndFocus() { setEnabled(false); }

			void select(DropdownOption* opt);
			ENGINE_INLINE auto getSelected() noexcept { return selected; }
	};

	class Dropdown : public StringLine {
		public:
			using OnSelectionCallback = std::function<bool(DropdownOption*)>;

		private:
			friend class DropdownContent;
			DropdownContent* content;
			OnSelectionCallback onSelection;

		public:
			Dropdown(Context* context) : StringLine{context} {
				autoText("This is a dropdown");
				setWidth(getWidth() + getHeight());

				content = ctx->createPanel<DropdownContent>(ctx->getRoot());
				content->dropdown = this;
				content->setEnabled(false);
				content->setAutoSize(true);
				content->setLayout(new DirectionalLayout{Direction::Vertical, Align::Start, Align::Stretch, ctx->getTheme().sizes.pad1});
			}

			virtual ~Dropdown() {
				ctx->deferedDeletePanel(content);
			}

			void addOption(std::string_view text, uint64 id) {
				auto opt = ctx->createPanel<DropdownOption>(content);
				opt->id = id;
				opt->autoText(text);
			}

			void setOnSelection(OnSelectionCallback func) {
				onSelection = func;
			}
			
			virtual bool onBeginActivate() {
				const bool e = !content->isEnabled();
				content->setEnabled(e);

				if (e) {
					content->setMinWidth(getWidth());
					content->setPos(getPos() + glm::vec2{0, getHeight()});
					content->getParent()->addChild(content); // Force to front
				}

				return true;
			}
			
			virtual void onEndFocus() {
				content->setEnabled(false);
			}

			virtual void render() override {
				const auto& theme = ctx->getTheme();
				const auto sz = getSize();
				ctx->setColor({1,0,0,1});
				ctx->drawRect({0,0}, sz);

				const auto p = round(theme.sizes.pad1 * 1.5);
				const auto s = round(sz.y*1.21f - 2*p);
				const auto x0 = sz.x - s - p;

				ctx->setColor(theme.colors.foreground);
				if (content->isEnabled()) {
					ctx->drawPoly({
						{x0, sz.y - p}, {x0 + s, sz.y - p}, {x0 + s*0.5f, p}
					});
				} else {
					ctx->drawPoly({
						{x0, p}, {x0 + s, p}, {x0 + s*0.5f,  sz.y - p}
					});
				}

				if (auto sel = content->getSelected(); sel) {
					sel->StringLine::render();
				} else {
					StringLine::render();
				}
			}
	};
}

namespace Engine::UI {
	inline void DropdownContent::select(DropdownOption* opt) {
		if (dropdown->onSelection) {
			if (!dropdown->onSelection(opt)) { return; }
		}

		if (selected) { selected->selected = false; }
		selected = opt;
		if (opt) { opt->selected = true; }
	}
}

namespace Engine::UI { 
	inline bool DropdownOption::onBeginActivate() {
		reinterpret_cast<DropdownContent*>(getParent())->select(this);
		return true;
	}
}
