#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/Panel.hpp>


namespace Engine::Gui {
	enum class Direction {
		Horizontal,
		Vertical,
		_count,
	};

	enum class Align {
		Start,
		End,
		Center,
		Stretch,
		_count,
	};

	class DirectionalLayout : public Layout {
		private:
			/** The main axis */
			Direction dir = {};

			/** Alignment along the main axis */
			Align mainAlign = Align::Start;

			/** Alignment along the cross axis */
			Align crossAlign = {};

			/** How much of a gap to insert between panels */
			float32 gap = 8;

			float32 weight = 0.0f;

		public:
			DirectionalLayout(Direction dir, Align mainAlign, Align crossAlign, float32 gap = 8.0f)
				: dir{dir}, mainAlign{mainAlign}, crossAlign{crossAlign}, gap{gap} {
			}

			ENGINE_INLINE void setGap(float32 g) noexcept { gap = g; }
			ENGINE_INLINE auto getGap() const noexcept { return gap; }

			virtual void layout(Panel* panel) override {
				const auto main = static_cast<uint32>(dir);
				const auto cross = static_cast<uint32>(dir == Direction::Horizontal ? Direction::Vertical : Direction::Horizontal);
				const auto size = panel->getSize();

				auto next = &Panel::getNextSibling;
				auto curr = panel->getFirstChild();
				auto totalWeight = weight;
				auto count = 0;
				glm::vec2 cpos = panel->getPos();

				if (mainAlign == Align::Start) {
					// Already set correctly
				} else if (mainAlign == Align::End) {
					cpos[main] += panel->getSize()[main];
					next = &Panel::getPrevSibling;
					curr = panel->getLastChild();
				} else if (mainAlign == Align::Center) {
					ENGINE_WARN("TODO: impl Align::Center for main axis");
				} else if (mainAlign == Align::Stretch) {
					// Already set correctly

					if (totalWeight == 0) {
						const auto old = curr;
						while (curr) {
							++count;
							totalWeight += curr->getWeight();
							curr = (curr->*next)();
						}
						curr = old;
					}
				} else [[unlikely]] {
					ENGINE_WARN("Unknown layout main axis alignment");
				}

				//auto advancePos()
				// TODO: could we pull some of the switching out of the loop? lambda? how does that compile down?
				while (curr) {
					auto pos = cpos;


					// TODO: refactor main/cross align so we dont resize twice
					if (mainAlign == Align::Stretch) {
						const auto w = curr->getWeight() / totalWeight;

						auto sz = curr->getSize();
						sz[main] = w * size[main] - (count > 1 ? gap : 0);
						curr->setSize(sz);
					}

					if (crossAlign == Align::Stretch) {
						auto sz = curr->getSize();
						sz[cross] = size[cross];
						curr->setSize(sz);
					}

					if (crossAlign == Align::Start) {
						// Already in correct pos
					} else if (crossAlign == Align::End) {
						pos[cross] += size[cross] - curr->getSize()[cross];
					} else if (crossAlign == Align::Center ||
					           crossAlign == Align::Stretch) {
						pos[cross] += (size[cross] - curr->getSize()[cross]) * 0.5f;
					} else [[unlikely]] {
						ENGINE_WARN("Unknown layout cross alignment.");
					}
					
					if (mainAlign == Align::End) {
						const auto diff = curr->getSize()[main] + gap;
						cpos[main] -= diff;
						pos[main] -= diff;
					} else {
						cpos[main] += curr->getSize()[main] + gap;
					}

					curr->setPos(pos);
					curr = (curr->*next)();
				}
			}
	};
}
