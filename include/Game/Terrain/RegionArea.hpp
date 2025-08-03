#pragma once

// Game
#include <Game/Terrain/RegionSpanX.hpp>


namespace Game::Terrain {
	/**
	 * Represents an AABB region area, in regions.
	 */
	class RegionArea {
		public:
			RegionVec min; // Inclusive
			RegionVec max; // Exclusive
			ENGINE_INLINE constexpr bool empty() const noexcept { return (min.x >= max.x) || (min.y >= max.y); }
			ENGINE_INLINE RegionSpanX toSpanX() const noexcept { return {min.x, max.x}; }
	};
}

template<>
struct fmt::formatter<Game::Terrain::RegionArea> {
	constexpr auto parse(format_parse_context& ctx) const {
		return ctx.end();
	}

	auto format(const Game::Terrain::RegionArea area, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "RegionArea({}, {})",
			area.min,
			area.max
		);
	}
};
