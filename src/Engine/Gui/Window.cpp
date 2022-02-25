// Engine
#include <Engine/Gui/Window.hpp>
#include <Engine/Gui/FillLayout.hpp>


namespace Engine::Gui {
	Window::Window(Context* context) : Panel{context} {
		setLayout(new FillLayout{outBorder});

		content = ctx->createPanel<Panel>(this);

		// TODO: probably shouldnt have a layout by default
		content->setLayout(new DirectionalLayout{Direction::Vertical, Align::Stretch, Align::Stretch, ctx->getTheme().sizes.pad1});

		title = ctx->createPanel<Title>(content, this);
		title->setFont(ctx->getTheme().fonts.body);
		title->autoText("Window Title");
		title->setRelPos({0, 0});
		title->setFixedHeight(title->getHeight() + 10);

		ctx->registerMouseMove(this, [this](glm::vec2 pos) { moveCallback(pos); });

		ctx->registerBeginActivate(this, [this](Panel* panel) { return beginActivateCallback(panel); });
				
		ctx->registerEndActivate(this, [this](Panel* panel) {
			if (!resizing) { return; }
			resizeDir = 0;
			resizing = false;

			if (hoverWithin) {
				updateResizeInfo(ctx->getCursor());
			} else {
				ctx->setCursor(Cursor::Normal);
			}
		});
	}

	void Window::moveCallback(const glm::vec2 pos) {
		if (tracking) {
			const auto bounds = getParent()->getBounds();
			const auto max = bounds.max - title->getSize();
			auto p = glm::clamp(pos - offset, bounds.min, max);
			setPos(p - outBorder);
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
			updateResizeInfo(pos);
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

	void Window::updateResizeInfo(const glm::vec2 pos) {
		const auto bounds = content->getBounds();

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

		ctx->setCursor(cur);
	}
}
