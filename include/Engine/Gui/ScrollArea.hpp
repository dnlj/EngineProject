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
			float32 s = 0;
			float32 p = 0;
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
				s = getSize()[D] * std::clamp(r, 0.0f, 1.0f);
			}

			ENGINE_INLINE void autoRatio(float32 areaSize) noexcept {
				setRatio(getSize()[D] / areaSize);
			}

			ENGINE_INLINE void setOnSlide(OnSlideCallback callback) {
				onSlide = std::move(callback);
			}

			virtual void onBeginActivate() {
				ctx->registerMouseMove(this, [this, init=p, last=ctx->getCursor()[D]](glm::vec2 pos) noexcept {
					p = init + pos[D] - last;
					const auto sz = getSize()[D];
					p = std::clamp(p, 0.0f, sz - s);
					if (onSlide && s < sz) { onSlide(p / (sz - s)); }
				});
			}

			virtual void onEndActivate() {
				ctx->deregisterMouseMove(this);
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
					virtual void render() { ctx->drawRect({}, getSize(), {1,0,1,1}); }
			};
			class DebugPanel final : public Panel {
				public:
					using Panel::Panel;
					virtual void render() { ctx->drawRect({}, getSize(), {0,0,1,1}); }
			};

		private:
			Panel* wrap = nullptr;
			ScrollPanel* content = nullptr;
			ScrollBarH* scrollX = nullptr;
			ScrollBarV* scrollY = nullptr;

		public:
			ScrollArea(Context* context, Direction dir = Direction::Vertical) : Panel{context} {
				setLayout(new GridLayout());
				wrap = ctx->createPanel<DebugPanel>(this);
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

			// TODO: mouse wheel support

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
