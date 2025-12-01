#pragma once

// Game
#include <Game/Terrain/RegionArea.hpp>


namespace Game::Terrain {
	/**
	 * Represents an AABB chunk area, in chunks.
	 */
	class ChunkArea {
		public:
			ChunkVec min; // Inclusive
			ChunkVec max; // Exclusive
			ENGINE_INLINE constexpr bool empty() const noexcept { return (min.x >= max.x) || (min.y >= max.y); }

			RegionArea toRegionArea() const noexcept {
				return {
					chunkToRegion(min),

					// Since max is exclusive we need to convert an inclusive range,
					// convert to region, then back to exclusive.
					chunkToRegion(max - ChunkVec{1,1}) + RegionVec{1,1}
				};
			}
	};

	inline void flattenRequests(std::vector<ChunkArea>& requests, std::vector<ChunkVec>& partitions) {
		std::ranges::sort(requests, [](const auto& a, const auto& b){ return a.min.x < b.min.x; });

		// Remove duplicates and total overlaps. Beyond that things can get more
		// complicated. We could be doing more to avoid all duplicate chunks in cases of
		// partial overlap, but that is quite a bit more involved so we will just rely on
		// the layers/generator to avoid re-generating duplicate data which they need to
		// do regardless.
		ChunkArea current = requests.front();
		current.max.x = --current.min.x;
		for (const auto& request : requests) {
			// Skip overlaps.
			if (request.max.x <= current.max.x && request.min.x <= current.max.x) {
				if (request.max.y <= current.max.y && request.min.y <= current.max.y) {
					continue;
				}
			}

			// Add the chunks in the request.
			for (auto chunkCoordX = request.min.x; chunkCoordX < request.max.x; ++chunkCoordX) {
				for (auto chunkCoordY = request.min.y; chunkCoordY < request.max.y; ++chunkCoordY) {
					partitions.push_back({chunkCoordX, chunkCoordY});
				}
			}
		}

		// This is just here for debugging. We _do_ actually expect duplicates here. See the above comment.
		if constexpr (ENGINE_DEBUG) {
			//std::ranges::sort(partitions,
			//	[](const ChunkVec& left, const ChunkVec& right) {
			//		if (left.x < right.x) { return true; }
			//		if (right.x < left.x) { return false; }
			//		if (left.y < right.y) { return true; }
			//		return false;
			//	}
			//);

			// Going all the way and totally avoiding duplicates does not seem beneficial,
			// especially considering we already have a step to remove already generated partitions
			// after this. Would need more rigorous and repeatable tests to justify doing this.
			//partitions.erase(std::unique(partitions.begin(), partitions.end()), partitions.end());
			
			//ENGINE_DEBUG_ASSERT(std::adjacent_find(partitions.cbegin(), partitions.cend()) == partitions.cend(),
			//	"Unexpected duplicate chunk area in request partition. This should not be possible."
			//);
		}
	}
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
