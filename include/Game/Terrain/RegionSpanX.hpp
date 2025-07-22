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

	
	inline void flattenRequests(std::vector<RegionSpanX>& requests, std::vector<RegionUnit>& partitions) {
		std::ranges::sort(requests, std::less{}, &RegionSpanX::min);

		// Insert every unique region in the ranges exactly once.
		auto last = requests.front().min - 1;
		for (const auto& req : requests) {
			for (RegionUnit region = std::max(last, req.min); region < req.max; ++region) {
				partitions.push_back(region);
			}

			last = std::max(last, req.max);
		}

		// These _should_ already be sorted, but double sort for debug safety.
		ENGINE_DEBUG_ONLY(std::ranges::sort(partitions));
		ENGINE_DEBUG_ASSERT(std::adjacent_find(partitions.cbegin(), partitions.cend()) == partitions.cend(),
			"Unexpected duplicate region in request partition. This should not be possible."
		);
	}
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
