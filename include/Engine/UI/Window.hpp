#pragma once

// Engine
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/Label.hpp>
#include <Engine/UI/Button.hpp>


namespace Engine::UI {
	class Window : public Panel {
		public:
			using CloseCallback = std::function<bool(Window*)>;

		private:
			class TitleBar : public Panel {
				public:
					Window* win;
					Label* title;
					Button* close;
					CloseCallback closeCallback;

				public:
					TitleBar(Context* context, Window* window);

					void setTitle(std::string_view txt) {
						title->autoText(txt);
					}

					virtual void render() override {
						ctx->setColor(ctx->getTheme().colors.title);
						ctx->drawRect({0,0}, getSize());
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
			TitleBar* title;
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

			ENGINE_INLINE void setTitle(const std::string_view text) { title->setTitle(text); }

			ENGINE_INLINE void setCloseCallback(CloseCallback func) {
				title->closeCallback = std::move(func);
			}

		private:
			void moveCallback(const glm::vec2 pos);
			void updateResizeInfo(const glm::vec2 pos);
	};
}
