#pragma once


namespace Game::Terrain {
	/**
	 * Represents a horizontal (X coord) span of block.
	 */
	class BlockSpanX {
		public:
			BlockUnit min; // Inclusive
			BlockUnit max; // Exclusive
			ENGINE_INLINE constexpr bool empty() const noexcept { return min >= max; }
	};
}

template<>
struct fmt::formatter<Game::Terrain::BlockSpanX> {
	constexpr auto parse(format_parse_context& ctx) const {
		return ctx.end();
	}

	auto format(const Game::Terrain::BlockSpanX area, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "BlockSpanX({}, {})", area.min, area.max);
	}
};
