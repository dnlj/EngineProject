#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/Panel.hpp>


namespace Engine::Gui {
	struct Direction_ {
		enum class Direction {
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

			void autoDim(Panel* panel, Direction axis) {
				int dim = static_cast<int>(axis);
				auto curr = panel->getFirstChild();
				float32 val = 0;

				if (axis != dir) { // Cross Axis
					while (curr) {
						val = std::max(val, curr->getSize()[dim]);
						curr = curr->getNextSibling();
					}
				} else { // Main Axis
					while (true) {
						val += curr->getSize()[dim]; 
						curr = curr->getNextSibling();
						if (!curr) { break; }
						val += curr ? gap : 0;
					}
				}

				auto sz = panel->getSize();
				sz[dim] = val;
				panel->setSize(sz);
				ENGINE_INFO(" ***** Auto Dim: ", dim, " ", sz);
			}

			virtual void autoHeight(Panel* panel) override {
				autoDim(panel, Direction::Vertical);
			}

			void autoWidth() {
				// TODO: autoDim(panel, Direction::Horizontal);
			}

			virtual void layout(Panel* panel) override {
				const auto main = static_cast<uint32>(dir);
				const auto cross = static_cast<uint32>(dir == Direction::Horizontal ? Direction::Vertical : Direction::Horizontal);
				const auto size = panel->getSize();

				auto next = &Panel::getNextSibling;
				auto curr = panel->getFirstChild();
				glm::vec2 cpos = panel->getPos();

				// Only used for main axis stretch.
				struct StretchData {
					float32 min = 0;
					float32 max = 0;
					float32 val = 0;
					float32 weight = 0;
				};
				// TODO: is there a better way to handle these single case variables?
				float32 totalWeight = weight;
				int32 count = 0;
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
					const bool accum = totalWeight == 0;
					while (curr) {
						++count;
						totalWeight += accum ? curr->getWeight() : 0;
						curr = curr->getNextSibling();
					}
					curr = panel->getFirstChild();

					stretchData.resize(count);
					for (int i = 0; curr; ++i, curr = curr->getNextSibling()) {
						stretchData[i] = {
							.min = curr->getMinSize()[main],
							.max = curr->getMaxSize()[main],
							.val = 0, // TODO: init to min, also subtract from remSize
							.weight = curr->getWeight(),
						};
					}
					curr = panel->getFirstChild();

					float32 remSize = size[main] - gap * (count - 1);
					float32 remWeight = totalWeight;

					// TODO: Also need to check that fixed totalWeight still works

					// TODO: seems like there may be an issue with needing extra iterations?
					// TODO: resize of an empty collapsible section should only take 1 iter? it says two.
					// TODO: resize of a slider(i think?) takes 5.
					// TODO: i think the above was fixed by the runWeight changes. verify.
					int iter = 0; // TODO: rm
					// While there is space remaining distribute it between all
					// non-max-size panels according to their relative remaining weight
					while (remSize >= 1 && remWeight > 0) {
						++iter;
						ENGINE_LOG("* i ", iter, " ", remSize);
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
					ENGINE_LOG("Iters: ", iter, " ", panel);
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
