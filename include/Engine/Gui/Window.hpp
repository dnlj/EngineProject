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

			int resizeDir = 0; // 1 = top, then clockwise
			bool resizing = false;
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

				ctx->registerMouseMove(this, [this](glm::vec2 pos) {
					if (tracking) {
						const auto bounds = getParent()->getBounds();
						const auto max = bounds.max - title->getSize();
						auto p = glm::clamp(pos - offset, bounds.min, max);
						setPos(p);
					} else if (resizing) {
						auto bounds = getBounds();
						const auto posAdj = pos + offset;
						switch (resizeDir) {
							//case 0: { break; }
							case 1: {
								bounds.min.y = posAdj.y;
								break;
							}
							case 2: {
								bounds.max.x = posAdj.x;
								bounds.min.y = posAdj.y;
								break;
							}
							case 3: {
								bounds.max.x = posAdj.x;
								break;
							}
							case 4: {
								bounds.max.x = posAdj.x;
								bounds.max.y = posAdj.y;
								break;
							}
							case 5: {
								bounds.max.y = posAdj.y;
								break;
							}
							case 6: {
								bounds.min.x = posAdj.x;
								bounds.max.y = posAdj.y;
								break;
							}
							case 7: {
								bounds.min.x = posAdj.x;
								break;
							}
							case 8: {
								bounds.min.x = posAdj.x;
								bounds.min.y = posAdj.y;
								break;
							}
						}

						setBounds(bounds);
					} else {
						const auto bounds = getBounds();
						constexpr auto outBorder = 5;
						constexpr auto inBorder = 2;

						auto cur = Cursor::Normal;

						const auto left = pos.x >= bounds.min.x - outBorder && pos.x <= bounds.min.x + inBorder;
						const auto right = pos.x <= bounds.max.x + outBorder && pos.x >= bounds.max.x - inBorder;
						const auto top = pos.y >= bounds.min.y - outBorder && pos.y <= bounds.min.y + inBorder;
						const auto bottom = pos.y <= bounds.max.y + outBorder && pos.y >= bounds.max.y - inBorder;

						if (left) {
							if (top) {
								cur = Cursor::Resize_TL_BR;
								resizeDir = 8;
							} else if (bottom) {
								cur = Cursor::Resize_BL_TR;
								resizeDir = 6;
							} else {
								cur = Cursor::Resize_L_R;
								resizeDir = 7;
							}
						} else if (right) {
							if (top) {
								cur = Cursor::Resize_BL_TR;
								resizeDir = 2;
							} else if (bottom) {
								cur = Cursor::Resize_TL_BR;
								resizeDir = 4;
							} else {
								cur = Cursor::Resize_L_R;
								resizeDir = 3;
							}
						} else if (top) {
							cur = Cursor::Resize_T_B;
							resizeDir = 1;
						} else if (bottom) {
							cur = Cursor::Resize_T_B;
							resizeDir = 5;
						} else {
							resizeDir = 0;
						}

						// TODO: atm this will override all other panels trying to set cursor. Go back to panel -> cursor association so we can just unset this panel.
						ctx->setCursor(cur);
					}
				});

				ctx->registerBeginActivate(this, [this](Panel* panel) -> bool {
					if (!resizeDir) { return false; }
					resizing = true;
					const auto bounds = getBounds();
					const auto pos = ctx->getCursor();
					switch (resizeDir) {
						case 1: {
							offset.y = bounds.min.y - pos.y;
							break;
						}
						case 2: {
							offset.x = bounds.max.x - pos.x;
							offset.y = bounds.min.y - pos.y;
							break;
						}
						case 3: {
							offset.x = bounds.max.x - pos.x;
							break;
						}
						case 4: {
							offset.x = bounds.max.x - pos.x;
							offset.y = bounds.max.y - pos.y;
							break;
						}
						case 5: {
							offset.y = bounds.max.y - pos.y;
							break;
						}
						case 6: {
							offset.x = bounds.min.x - pos.x;
							offset.y = bounds.max.y - pos.y;
							break;
						}
						case 7: {
							offset.x = bounds.min.x - pos.x;
							break;
						}
						case 8: {
							offset.x = bounds.min.x - pos.x;
							offset.y = bounds.min.y - pos.y;
							break;
						}
					}
					return true;
				});
				
				ctx->registerEndActivate(this, [this](Panel* panel) {
					if (!resizing) { return; }
					resizeDir = 0;
					resizing = false;
					ctx->setCursor(Cursor::Normal);
				});
			}
	};
}
