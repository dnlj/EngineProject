#pragma once


namespace Game::Terrain {
	/**
	 * Represents a horizontal (X coord) span of regions, in regions.
	 */
	class RegionSpanX {
		public:
			RegionUnit min; // Inclusive
			RegionUnit max; // Exclusive
			ENGINE_INLINE constexpr bool empty() const noexcept { return min >= max; }
	};
}

template<>
struct fmt::formatter<Game::Terrain::RegionSpanX> {
	constexpr auto parse(format_parse_context& ctx) const {
		return ctx.end();
	}

	auto format(const Game::Terrain::RegionSpanX area, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "RegionSpanX({}, {}) = BlockArea({}, {})",
			area.min,
			area.max,
			area.min * Game::chunksPerRegion * Game::blocksPerChunk,
			area.max * Game::chunksPerRegion * Game::blocksPerChunk
		);
	}
};
