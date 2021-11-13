#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/Panel.hpp>


namespace Engine::Gui {
	struct Direction_ {
		enum Direction {
			Horizontal,
			Vertical,
			_count,
		};
	};
	using Direction = Direction_::Direction;

	struct Align_ {
		enum Align {
			Start,
			End,
			Center,
			Stretch,
			_count,
		};
	};
	using Align = Align_::Align;

	class DirectionalLayout : public Layout {
		protected:
			/** The main axis */
			Direction dir = {};

			/** Alignment along the main axis */
			Align mainAlign = Align::Start;

			/** Alignment along the cross axis */
			Align crossAlign = {};

			/** How much of a gap to insert between panels */
			float32 gap = 8;

			/** Total main axis stretch weight. Zero for automatic weight. */
			float32 weight = 0.0f;

			bool autoSize = false;

		public:
			DirectionalLayout(Direction dir, Align mainAlign, Align crossAlign, float32 gap = 8.0f, float32 weight = 0.0f)
				: dir{dir}, mainAlign{mainAlign}, crossAlign{crossAlign}, gap{gap} {
			}

			ENGINE_INLINE void setGap(float32 g) noexcept { gap = g; }
			ENGINE_INLINE auto getGap() const noexcept { return gap; }

			ENGINE_INLINE void setWeight(float32 w) noexcept { weight = w; }
			ENGINE_INLINE auto getWeight() const noexcept { return weight; }

			float32 getAutoDim(const Panel* panel, Direction axis) const {
				int dim = static_cast<int>(axis);
				auto curr = panel->getFirstChild();
				float32 val = 0;

				if (axis != dir) { // Cross Axis
					while (curr) {
						val = std::max(val, curr->getSize()[dim]);
						curr = curr->getNextSibling();
					}
				} else { // Main Axis
					if (curr) {
						while (true) {
							val += curr->getSize()[dim]; 
							curr = curr->getNextSibling();
							if (!curr) { break; }
							val += gap;
						}
					}
				}

				return val;
			}

			virtual float32 getAutoHeight(const Panel* panel) const override {
				return getAutoDim(panel, Direction::Vertical);
			}

			virtual float32 getAutoWidth(const Panel* panel) const override {
				return getAutoDim(panel, Direction::Horizontal);
			}

			virtual void layout(Panel* panel) override {
				const auto main = dir;
				const auto cross = dir == Direction::Horizontal ? Direction::Vertical : Direction::Horizontal;
				const auto size = panel->getSize();

				auto next = &Panel::getNextSibling;
				auto curr = panel->getFirstChild();
				glm::vec2 cpos = panel->getPos();

				struct StretchData {
					float32 min = 0;
					float32 max = 0;
					float32 val = 0;
					float32 weight = 0;
				};
				
				// Only used for main axis stretch.
				int stretchIndex = -1;
				std::vector<StretchData> stretchData;

				if (mainAlign == Align::Start) {
					// Already set correctly
				} else if (mainAlign == Align::End) {
					cpos[main] += panel->getSize()[main];
					next = &Panel::getPrevSibling;
					curr = panel->getLastChild();
				} else if (mainAlign == Align::Center) {
					ENGINE_WARN("TODO: impl Align::Center for main axis");
				} else if (mainAlign == Align::Stretch) {
					float32 remWeight = 0;
					int32 count = 0;
					while (curr) {
						++count;
						remWeight += curr->getWeight();
						curr = curr->getNextSibling();
					}
					curr = panel->getFirstChild();

					stretchData.resize(count);
					for (int i = 0; curr; ++i, curr = curr->getNextSibling()) {
						stretchData[i] = {
							.min = curr->getMinSize()[main],
							.max = curr->getMaxSize()[main],
							.val = 0,
							.weight = curr->getWeight(),
						};
					}
					curr = panel->getFirstChild();
					
					float32 remSize = size[main];
					if (weight) { remSize *= remWeight / weight; }
					remSize -= gap * (count - 1);

					// While there is space remaining distribute it between all
					// non-max-size panels according to their relative remaining weight
					while (remSize >= 1 && remWeight > 0) {
						float32 runWeight = remWeight;
						for (auto& d : stretchData) {
							if (d.val == d.max) { continue; }
							float32 w = d.weight / runWeight;
							float32 v = w * remSize;
							float32 b = d.val;
							d.val = std::clamp(d.val + v, d.min, d.max);
							remSize -= d.val - b;
							runWeight -= d.weight;

							if (d.val == d.max) {
								remWeight -= d.weight;
							}
						}
					}
				} else [[unlikely]] {
					ENGINE_WARN("Unknown layout main axis alignment");
				}

				// TODO: could we pull some of the switching out of the loop? lambda? how does that compile down?
				const auto stretch = mainAlign == Align::Stretch || crossAlign == Align::Stretch;
				while (curr) {
					auto pos = cpos;

					if (stretch) {
						auto sz = curr->getSize();

						if (mainAlign == Align::Stretch) {
							// Use pre build data
							sz[main] = stretchData[++stretchIndex].val;
						}

						if (crossAlign == Align::Stretch) {
							sz[cross] = size[cross];
						}

						curr->setSize(sz);
					}
					
					if (mainAlign == Align::End) {
						const auto diff = curr->getSize()[main] + gap;
						cpos[main] -= diff;
						pos[main] -= diff;
					} else {
						cpos[main] += curr->getSize()[main] + gap;
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
