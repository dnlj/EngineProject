#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>
#include <Engine/Gui/common.hpp>
#include <Engine/Gui/GridLayout.hpp>


namespace Engine::Gui {
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
				auto sz = getSize();
				ctx->drawRect({}, sz, {0,1,0,1});

				glm::vec2 pos = {};
				pos[D] = p;
				sz[D] = s;

				ctx->drawRect(pos, sz, {1,0,0,1});
			}

			void setRatio(float32 r) noexcept {
				// TODO: need to handle minimum size
				s = std::round(getSize()[D] * std::clamp(r, 0.0f, 1.0f));
			}

			ENGINE_INLINE void autoRatio(float32 areaSize) noexcept {
				setRatio(getSize()[D] / areaSize);
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

			virtual void onBeginActivate() {
				ctx->registerMouseMove(this, [this, init=p, last=ctx->getCursor()[D]](glm::vec2 pos) noexcept {
					setScrollOffset(init + pos[D] - last);
				});
			}

			virtual void onEndActivate() {
				ctx->deregisterMouseMove(this);
			}

			virtual void onAction(ActionEvent action) override {
				if (D == Direction::Vertical && action == Action::Scroll) {
					auto off = ctx->getTheme().fonts.body->getLineHeight() * action.value.f32 * ctx->getScrollLines();
					setScrollOffset(p - off);
				} else if (D == Direction::Horizontal && action == Action::ScrollH) {
					ENGINE_WARN("TODO: impl"); // TODO; impl
				}
			}
	};

	using ScrollBarH = ScrollBar<Direction::Horizontal>;
	using ScrollBarV = ScrollBar<Direction::Vertical>;

	class ScrollArea : public Panel {
		private:
			class ScrollPanel final : public Panel {
				public:
					ScrollArea* area;
					using Panel::Panel;
					virtual void postLayout() override { area->updateScrollArea(); }
					//virtual void render() { ctx->drawRect({}, getSize(), {1,0,1,1}); }
			};

		private:
			Panel* wrap = nullptr;
			ScrollPanel* content = nullptr;
			ScrollBarH* scrollX = nullptr;
			ScrollBarV* scrollY = nullptr;

		public:
			ScrollArea(Context* context, Direction dir = Direction::Vertical) : Panel{context} {
				setLayout(new GridLayout());
				wrap = ctx->createPanel<Panel>(this);
				content = ctx->createPanel<ScrollPanel>(wrap);
				content->area = this;
				setDirection(dir);
			}

			void updateScrollArea() {
				if (scrollX) {
					scrollX->autoRatio(content->getWidth());
				}
				if (scrollY) {
					scrollY->autoRatio(content->getHeight());
				}
			}

			// TODO: this wont work because actions are not propegated backward. Add bool (consume) return value.
			virtual void onAction(ActionEvent action) override {
				switch (action) {
					case Action::Scroll:
					case Action::ScrollH: {
						if (scrollX) { scrollX->onAction(action); }
						if (scrollY) { scrollY->onAction(action); }
					}
				}
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
							content->setPosX(getPosX() - getWidth() * p);
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
						scrollY->setOnSlide([this](float32 p){
							content->setPosY(getPosY() - getHeight() * p);
						});
					}
				}
			}

			ENGINE_INLINE Panel* getContent() const noexcept { return content; }
			ENGINE_INLINE Panel* getContent() noexcept { return content; }
	};
}