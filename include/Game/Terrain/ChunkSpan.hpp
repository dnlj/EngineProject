#pragma once


namespace Game::Terrain {
	/**
	 * Represents a horizontal (X coord) span of chunks, in chunks.
	 */
	class ChunkSpanX {
		public:
			ChunkUnit min; // Inclusive
			ChunkUnit max; // Exclusive
			ENGINE_INLINE constexpr bool empty() const noexcept { return min >= max; }
	};
}

template<>
struct fmt::formatter<Game::Terrain::ChunkSpanX> {
	constexpr auto parse(format_parse_context& ctx) const {
		return ctx.end();
	}

	auto format(const Game::Terrain::ChunkSpanX area, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "ChunkSpanX({}, {}) = BlockArea({}, {})",
			area.min,
			area.max,
			area.min * Game::blocksPerChunk,
			area.max * Game::blocksPerChunk
		);
	}
};
