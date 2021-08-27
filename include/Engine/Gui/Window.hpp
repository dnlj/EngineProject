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
						win->offset = win->getContext()->getCursor() - win->getPos() - outBorder;
					}

					virtual void onEndActivate() override {
						win->tracking = false;
					}

					virtual bool canHover() const override { return true; }
					virtual bool canFocus() const override { return true; }
			};
			
			constexpr static float32 outBorder = 5;
			constexpr static float32 inBorder = 2; // TODO: i dont think we really want an in border. just tall corners.

			int resizeDir = 0; // 1 = top, then clockwise
			bool resizing = false;
			bool tracking = false;
			bool hoverWithin = false;
			glm::vec2 offset = {};
			Title* title;
			Panel* main;

		public:
			Window(Context* context);
			
			virtual void onBeginHover() override { hoverWithin = true; };
			virtual void onEndHover() override {
				hoverWithin = false;
				if (!resizing) { ctx->setCursor(Cursor::Normal); }
			};

			virtual void onBeginChildHover(Panel* child) override { hoverWithin = true; };
			virtual void onEndChildHover(Panel* child) override {
				hoverWithin = false;
				if (!resizing) { ctx->setCursor(Cursor::Normal); }
			};

		private:
			void moveCallback(const glm::vec2 pos);
			bool beginActivateCallback(Panel* panel);
			void updateResizeInfo(const glm::vec2 pos);
	};
}
