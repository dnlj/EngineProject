#pragma once


namespace Game::Terrain {
	/**
	 * Represents an AABB chunk area, in chunks.
	 */
	class ChunkArea {
		public:
			ChunkVec min; // Inclusive
			ChunkVec max; // Exclusive
			ENGINE_INLINE constexpr bool empty() const noexcept { return (min.x >= max.x) || (min.y >= max.y); }
	};
}

template<>
struct fmt::formatter<Game::Terrain::ChunkArea> {
	constexpr auto parse(format_parse_context& ctx) const {
		return ctx.end();
	}

	auto format(const Game::Terrain::ChunkArea area, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "ChunkArea({}, {}) = BlockArea({}, {})",
			area.min,
			area.max,
			area.min * Game::blocksPerChunk,
			area.max * Game::blocksPerChunk
		);
	}
};
