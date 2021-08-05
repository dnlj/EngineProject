#pragma once

// Engine
#include <Engine/Engine.hpp>


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

	class DirectionalLayout {
		private:
			using LayoutFunc = void(DirectionalLayout::*)(Panel*);

			Direction dir = {};
			Align align = {};

			LayoutFunc layoutFunc = nullptr;

			float32 gap = 8;

		public:
			DirectionalLayout(Direction dir, Align align)
				: dir{dir}, align{align} {
				updateDispatch();
			}

			ENGINE_INLINE decltype(auto) layout(Panel* panel) {
				ENGINE_DEBUG_ASSERT(layoutFunc, "Invalid layout function");
				return (this->*layoutFunc)(panel);
			}

		private:
			template<Direction D, Align A>
			void layoutImpl(Panel* panel) {
				auto curr = panel->getChildList();
				auto size = panel->getSize();
				auto cpos = panel->getPos();
				cpos += gap;

				while (curr) {
					curr->setPos(cpos);

					if constexpr (A == Align::Start) {
					} else if constexpr (A == Align::End) {
					} else if constexpr (A == Align::Center) {
					} else if constexpr (A == Align::Stretch) {
					} else {
						static_assert(A != A, "Unknown Alignment");
					}

					if constexpr (D == Direction::Horizontal) {
						cpos.y += curr->getSize().x + gap;
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
