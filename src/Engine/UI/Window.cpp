// Engine
#include <Engine/UI/Window.hpp>
#include <Engine/UI/FillLayout.hpp>


namespace Engine::UI {
	Window::TitleBar::TitleBar(Context* context, Window* window) : Panel{context}, win{window} {
		const auto& theme = ctx->getTheme();

		const auto h = theme.fonts.body->getLineHeight();
		setFixedHeight(h);

		title = ctx->constructPanel<Label>();
		title->setFont(theme.fonts.body);
		title->setPadding(0);
		title->setFixedHeight(h);

		close = ctx->constructPanel<Button>();
		close->setFont(theme.fonts.body);
		close->setPadding(0);
		close->autoText("Close");
		close->setFixedHeight(h);
		close->setFixedWidth(close->getWidth());
		close->setAction([this](Button*){
			if (closeCallback) { closeCallback(win); }
		});

		addChildren({title, close});
		setLayout(new DirectionalLayout{Direction::Horizontal, Align::Stretch, Align::Center, 0, 0});
	}

	Window::Window(Context* context) : Panel{context} {
		setLayout(new FillLayout{outBorder});

		area = ctx->createPanel<Panel>(this);
		area->setLayout(new DirectionalLayout{Direction::Vertical, Align::Stretch, Align::Stretch, ctx->getTheme().sizes.pad1});

		title = ctx->createPanel<TitleBar>(area, this);

		content = ctx->createPanel<PanelT>(area);
		content->setLayout(new DirectionalLayout{Direction::Vertical, Align::Start, Align::Stretch, ctx->getTheme().sizes.pad1});

		ctx->registerMouseMove(this, [this](glm::vec2 pos) { moveCallback(pos); });
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

	bool Window::onBeginActivate() {
		if (!resizeDir) { return true; }

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

	void Window::onEndActivate() {
		if (!resizing) { return; }
		resizeDir = 0;
		resizing = false;

		if (hoverWithin) {
			updateResizeInfo(ctx->getCursor());
		} else {
			ctx->setCursor(Cursor::Normal);
		}
	}

	void Window::updateResizeInfo(const glm::vec2 pos) {
		const auto bounds = area->getBounds();

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
