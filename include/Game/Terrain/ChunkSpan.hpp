#pragma once

// Game
#include <Game/Terrain/RegionSpanX.hpp>


namespace Game::Terrain {
	/**
	 * Represents a horizontal (X coord) span of chunks, in chunks.
	 */
	class ChunkSpanX {
		public:
			ChunkUnit min; // Inclusive
			ChunkUnit max; // Exclusive

			ENGINE_INLINE constexpr bool operator<(const ChunkSpanX& right) const noexcept { return (min < right.min) && (max < right.max); }
			ENGINE_INLINE constexpr bool operator==(const ChunkSpanX&) const noexcept = default;
			ENGINE_INLINE constexpr bool empty() const noexcept { return min >= max; }

			ENGINE_INLINE constexpr RegionSpanX toRegionSpan() const noexcept {
				// Since max is exclusive we need to convert an inclusive range,
				// convert to region, then back to exclusive.
				return {chunkToRegion({min, 0}).x, chunkToRegion({max - 1, 0}).x + 1};
			}
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
