#pragma once

// Engine
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/Label.hpp>


namespace Engine::UI {
	class Window : public Panel {
		private:
			class Title : public Label {
				public:
					Window* win;

				public:
					Title(Context* context, Window* window) : Label{context}, win{window} {}

					virtual void render() override {
						ctx->drawRect({0,0}, getSize(), ctx->getTheme().colors.title);
						Label::render();
					}
					
					virtual bool onBeginActivate() override {
						win->tracking = true;
						win->offset = win->getContext()->getCursor() - win->getPos() - outBorder;
						return true;
					}

					virtual void onEndActivate() override {
						win->tracking = false;
					}

					virtual bool canHover() const override { return true; }
					virtual bool canFocus() const override { return true; }
			};
			
			constexpr static float32 outBorder = Context::getResizeBorderSize().x;
			constexpr static float32 inBorder = outBorder; // Because of how hoverWithin works this is just corner height now.

			int resizeDir = 0; // 1 = top, then clockwise
			bool resizing = false;
			bool tracking = false;
			bool hoverWithin = false;
			glm::vec2 offset = {};
			Title* title;
			Panel* area;
			Panel* content;

		public:
			Window(Context* context);

			ENGINE_INLINE auto getContent() const noexcept { return content; }
			
			virtual void onBeginHover() override { hoverWithin = true; };
			virtual void onEndHover() override {
				hoverWithin = false;
				if (!resizing) {
					resizeDir = 0;
					ctx->setCursor(Cursor::Normal);
				}
			};

			virtual void onBeginChildHover(Panel* child) override { onEndHover(); };

			virtual bool onBeginActivate() override;
			virtual void onEndActivate() override;

			ENGINE_INLINE void setTitle(const std::string_view text) { title->autoText(text); }

		private:
			void moveCallback(const glm::vec2 pos);
			void updateResizeInfo(const glm::vec2 pos);
	};
}