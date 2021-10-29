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
			using LayoutFunc = void(DirectionalLayout::*)(Panel*);

			/** The main axis */
			Direction dir = {};

			/** Alignment along the cross axis */
			Align align = {};

			/** How much of a gap to insert between panels */
			float32 gap = 8;

			/** The function called when we perform layout. Depends on dispatch. */
			LayoutFunc layoutFunc = nullptr;

		public:
			DirectionalLayout(Direction dir, Align align, float32 gap = 8.0f)
				: dir{dir}, align{align}, gap{gap} {
				updateDispatch();
			}

			virtual void layout(Panel* panel) override {
				ENGINE_DEBUG_ASSERT(layoutFunc, "Invalid layout function");
				return (this->*layoutFunc)(panel);
			}

			ENGINE_INLINE void setGap(float32 g) noexcept { gap = g; }
			ENGINE_INLINE auto getGap() const noexcept { return gap; }

		private:
			template<Direction D, Align A>
			void layoutImpl(Panel* panel) {
				constexpr auto cross = static_cast<uint32>(D == Direction::Horizontal ? Direction::Vertical : Direction::Horizontal);
				const auto size = panel->getSize();

				auto curr = panel->getFirstChild();
				auto cpos = panel->getPos();

				while (curr) {
					auto pos = cpos;

					if constexpr (A == Align::Stretch) {
						auto sz = curr->getSize();
						sz[cross] = size[cross];
						curr->setSize(sz);
					}

					if constexpr (A == Align::Start) {
						// Already in correct pos
					} else if constexpr (A == Align::End) {
						pos[cross] += size[cross] - curr->getSize()[cross];
					} else if constexpr (A == Align::Center ||
					                     A == Align::Stretch) {
						pos[cross] += (size[cross] - curr->getSize()[cross]) * 0.5f;
					} else {
						static_assert(A != A, "Unknown Alignment");
					}

					curr->setPos(pos);

					if constexpr (D == Direction::Horizontal) {
						cpos.x += curr->getSize().x + gap;
					} else if constexpr (D == Direction::Vertical) {
						cpos.y += curr->getSize().y + gap;
					} else {
						static_assert(D != D, "Unknown Direction");
					}

					curr = curr->getNextSibling();
				}
			}

			void updateDispatch() {
				using D = std::underlying_type_t<Direction>;
				using A = std::underlying_type_t<Align>;
				constexpr auto dCount = static_cast<D>(Direction::_count);
				constexpr auto aCount = static_cast<D>(Align::_count);

				constexpr LayoutFunc lookup[dCount][aCount] = {
					{
						{&DirectionalLayout::template layoutImpl<Direction::Horizontal, Align::Start>},
						{&DirectionalLayout::template layoutImpl<Direction::Horizontal, Align::End>},
						{&DirectionalLayout::template layoutImpl<Direction::Horizontal, Align::Center>},
						{&DirectionalLayout::template layoutImpl<Direction::Horizontal, Align::Stretch>},
					},
					{
						{&DirectionalLayout::template layoutImpl<Direction::Vertical, Align::Start>},
						{&DirectionalLayout::template layoutImpl<Direction::Vertical, Align::End>},
						{&DirectionalLayout::template layoutImpl<Direction::Vertical, Align::Center>},
						{&DirectionalLayout::template layoutImpl<Direction::Vertical, Align::Stretch>},
					}
				};

				constexpr bool lookup_check = [&]() {
					for (D d = {}; d < dCount; ++d) {
						for (A a = {}; a < aCount; ++a) {
							if (lookup[d][a] == nullptr) {
								throw "Missing lookup for one or more enum combination";
							}
						}
					}
					return false;
				}();

				if ((dir < Direction{} || dir > Direction::_count) ||
					(align < Align{} || align > Align::_count)) {

					layoutFunc = lookup[0][0];
					ENGINE_WARN("Invalid directional layout");
				}

				layoutFunc = lookup[static_cast<D>(dir)][static_cast<A>(align)];
			}
	};
}
