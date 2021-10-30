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
				glm::vec2 cpos = panel->getPos();

				// Only used for main axis stretch.
				// TODO: is there a better way to handle these single case variables?
				float32 totalWeight = weight;
				int32 count = 0;
				float32 gapAdj = 0;

				if (mainAlign == Align::Start) {
					// Already set correctly
				} else if (mainAlign == Align::End) {
					cpos[main] += panel->getSize()[main];
					next = &Panel::getPrevSibling;
					curr = panel->getLastChild();
				} else if (mainAlign == Align::Center) {
					ENGINE_WARN("TODO: impl Align::Center for main axis");
				} else if (mainAlign == Align::Stretch) {
					const bool accum = totalWeight == 0;
					const auto old = curr;
					while (curr) {
						++count;
						totalWeight += accum ? curr->getWeight() : 0;
						curr = (curr->*next)();
					}
					curr = old;
					gapAdj = gap * (count - 1) / count;
				} else [[unlikely]] {
					ENGINE_WARN("Unknown layout main axis alignment");
				}

				// TODO: could we pull some of the switching out of the loop? lambda? how does that compile down?
				while (curr) {
					auto pos = cpos;

					// TODO: refactor main/cross align so we dont resize twice
					if (mainAlign == Align::Stretch) {
						// This is arguably incorrect because we distribute the gap space
						// evenly between all panels. This causes things to be slightly off
						// from what you might expect. For example with two panels 2:1
						// the panel sizes themself will not be quite 2:1. It is the total
						// `panelSize + gap*(n-1)/n` which is 2:1.
						//
						// The other way to do things would be to take the total space then
						// subtract (n-1)*gap from it then divide that between all panels.
						// You would also need to use this new gap adjusted size to calc
						// individual panels weight adjusted size. Overall this makes more
						// intuitive sense but isnt necessarily more correct.
						//
						// The second way (removing gaps before dividing size) doesnt work
						// well with fixed layout weighting, whereas the first does.
						//
						const auto w = curr->getWeight() / totalWeight;
						auto sz = curr->getSize();
						sz[main] = w * size[main] - gapAdj;
						curr->setSize(sz);
					}
					
					if (mainAlign == Align::End) {
						const auto diff = curr->getSize()[main] + gap;
						cpos[main] -= diff;
						pos[main] -= diff;
					} else {
						cpos[main] += curr->getSize()[main] + gap;
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

					curr->setPos(pos);
					curr = (curr->*next)();
				}
			}
	};
}
