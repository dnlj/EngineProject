// Engine
#include <Engine/Gui/Window.hpp>


namespace {
	// TODO: Could probably be handled by directional layout - stretch items or similar
	struct FillLayout : Engine::Gui::Layout {
		int pad = 0;

		FillLayout(int padding) : pad{padding} {}
		virtual void layout(Engine::Gui::Panel* panel) override {
			auto* child = panel->getFirstChild();
			auto bounds = panel->getBounds();
			bounds.min += pad;
			bounds.max -= pad;
			child->setBounds(bounds);
		}
	};
}


namespace Engine::Gui {
	Window::Window(Context* context) : Panel{context} {
		setLayout(new FillLayout{outBorder});

		main = ctx->createPanel<Panel>();
		addChild(main);
		main->setLayout(new DirectionalLayout{Direction::Vertical, Align::Stretch});

		title = ctx->createPanel<Title>(this);
		main->addChild(title);
		title->setFont(ctx->font_b);
		title->setText("Window Title");
		title->setRelPos({0, 0});
		title->autoSize();


		ctx->registerMouseMove(this, [this](glm::vec2 pos) { moveCallback(pos); });

		ctx->registerBeginActivate(this, [this](Panel* panel) { return beginActivateCallback(panel); });
				
		ctx->registerEndActivate(this, [this](Panel* panel) {
			if (!resizing) { return; }
			resizeDir = 0;
			resizing = false;
			ctx->setCursor(Cursor::Normal);
		});
	}

	void Window::moveCallback(const glm::vec2 pos) {
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
		} else if (hoverWithin) {
			const auto bounds = main->getBounds();

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
	}

	bool Window::beginActivateCallback(Panel* panel) {
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
	}
}
