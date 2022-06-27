#pragma once

// Engine
#include <Engine/Gui/common.hpp>
#include <Engine/Gui/Context.hpp>
#include <Engine/Gui/FillLayout.hpp>
#include <Engine/Gui/GridLayout.hpp>
#include <Engine/Gui/Panel.hpp>


namespace Engine::UI {
	template<Direction D>
	class ScrollBar : public Panel {
		public:
			using OnSlideCallback = std::function<void(float32)>;

		private:
			float32 s = 0; // Bar size
			float32 p = 0; // Position offset
			OnSlideCallback onSlide;

		public:
			ScrollBar(Context* context) : Panel{context} {
				setWeight(0);
				setFixedWidth(ctx->getTheme().sizes.scrollWidth);
			}

			void render() {
				auto& theme = ctx->getTheme();
				auto sz = getSize();
				ctx->drawRect({}, sz, theme.colors.feature);

				glm::vec2 pos = {};
				pos[D] = p;
				sz[D] = s;

				ctx->drawRect(pos, sz, theme.colors.accent);
			}

			void setRatio(float32 r) noexcept {
				// TODO: need to handle minimum size
				const auto sz = getSize()[D];
				s = std::round(sz * std::clamp(r, 0.0f, 1.0f));
				if (const auto max = sz - s; p > max) { setScrollOffset(max); }
			}

			ENGINE_INLINE void setOnSlide(OnSlideCallback callback) {
				onSlide = std::move(callback);
			}

			void setScrollOffset(float32 off) {
				const auto sz = getSize()[D];
				const auto n = std::clamp(off, 0.0f, getSize()[D] - s);

				if (n != p && onSlide && s < sz) {
					p = n;
					onSlide(p / (sz - s));
				}
			}

			virtual bool onBeginActivate() {
				const auto abs = getPos()[D];
				const auto cur = ctx->getCursor()[D];
				if (cur < abs + p || cur > abs + p + s) {
					setScrollOffset(cur - abs - s * 0.5f);
				}

				ctx->registerMouseMove(this, [this, init=p, last=cur](glm::vec2 pos) noexcept {
					setScrollOffset(init + pos[D] - last);
				});

				return true;
			}

			virtual void onEndActivate() {
				ctx->deregisterMouseMove(this);
			}

			virtual bool onAction(ActionEvent action) override {
				if (D == Direction::Vertical && action == Action::Scroll) {
					auto off = ctx->getTheme().fonts.body->getLineHeight() * action.value.f32 * ctx->getScrollLines();
					setScrollOffset(p - off);
					return true;
				} else if (D == Direction::Horizontal && action == Action::ScrollH) {
					ENGINE_WARN("TODO: impl"); // TODO; impl
					return true;
				}
				return false;
			}
	};

	using ScrollBarH = ScrollBar<Direction::Horizontal>;
	using ScrollBarV = ScrollBar<Direction::Vertical>;

	class ScrollArea : public Panel {
		private:
			class ScrollPanel final : public PanelT {
				public:
					ScrollArea* area;
					using PanelT::PanelT;
					virtual void postLayout() override { area->updateScrollArea(); }

					virtual void render() override {
						ctx->setClip({getPos(), getPos() + getSize()});
					}
			};

		private:
			Panel* wrap = nullptr;
			ScrollPanel* content = nullptr;
			ScrollBarH* scrollX = nullptr;
			ScrollBarV* scrollY = nullptr;

		public:
			ScrollArea(Context* context, Direction dir = Direction::Vertical) : Panel{context} {
				setLayout(new GridLayout());
				wrap = ctx->createPanel<PanelT>(this);
				content = ctx->createPanel<ScrollPanel>(wrap);
				content->area = this;
				setDirection(dir);
			}

			virtual void render() override {
				ctx->setClip({getPos(), getPos() + getSize()});
			}

			void updateScrollArea() {
				if (scrollX) {
					scrollX->setRatio(wrap->getWidth() / content->getWidth());
				}
				if (scrollY) {
					scrollY->setRatio(wrap->getHeight() / content->getHeight());
				}
			}

			virtual bool onAction(ActionEvent action) override {
				switch (action) {
					case Action::Scroll:
					case Action::ScrollH: {
						if (scrollX) { scrollX->onAction(action); }
						if (scrollY) { scrollY->onAction(action); }
						return true;
					}
				}
				return false;
			}

			virtual void postLayout() override {
				updateScrollArea();
			}

			void setDirection(Direction d) {
				content->setAutoSize(false);
				if (d == Direction::Horizontal) {
					wrap->setLayout(new StretchLayout<Direction::Vertical>{0});
					content->setAutoSizeWidth(true);
					if (scrollY) {
						ctx->deletePanel(scrollY);
						setRelPos({});
					}
					if (!scrollX) {
						scrollX = ctx->createPanel<ScrollBarH>(this);
						scrollX->setGridPos(0,1);
						scrollX->setOnSlide([this](float32 p){
							const auto r = 1 - wrap->getWidth() / content->getWidth();
							content->setPosX(wrap->getPosX() - content->getWidth() * p * r);
						});
					}
				} else if (d == Direction::Vertical) {
					wrap->setLayout(new StretchLayout<Direction::Horizontal>{0});
					content->setAutoSizeHeight(true);
					if (scrollX) {
						ctx->deletePanel(scrollX);
						setRelPos({});
					}
					if (!scrollY) {
						scrollY = ctx->createPanel<ScrollBarV>(this);
						scrollY->setGridPos(1,0);
						scrollY->setOnSlide([this](float32 p) {
							const auto r = 1 - wrap->getHeight() / content->getHeight();
							content->setPosY(wrap->getPosY() - content->getHeight() * p * r);
						});
					}
				}
			}

			ENGINE_INLINE Panel* getContent() const noexcept { return content; }
			ENGINE_INLINE Panel* getContent() noexcept { return content; }
	};
}
