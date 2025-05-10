#pragma once


namespace Game::Terrain {
	/**
	 * Store data per chunk for each region.
	 */
	template<class ChunkData>
	class RegionDataCache {
		private:
			using Store = RegionStore<ChunkData>;
			Engine::FlatHashMap<RegionVec, std::unique_ptr<Store>> regions;

		public:
			RegionDataCache() = default;
			RegionDataCache(RegionDataCache&&) = default;
			RegionDataCache(const RegionDataCache&) = delete;

			ENGINE_INLINE Store& at(RegionVec regionCoord) noexcept {
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end(), "Attempting to access region outside of RegionDataCache.");
				return *found->second;
			}

			ENGINE_INLINE const Store& at(RegionVec regionCoord) const noexcept {
				return const_cast<RegionDataCache*>(this)->at(regionCoord);
			}

			ENGINE_INLINE void reserve(RegionVec regionCoord) noexcept {
				const auto found = regions.find(regionCoord);
				if (found == regions.end()) {
					regions.try_emplace(regionCoord, std::make_unique<Store>());
				}
			}

			// TODO: Function sig concept
			ENGINE_INLINE void forEachChunk(ChunkArea area, auto&& func) {
				// TODO: Could be a bit more effecient by dividing into regions first. Then you
				//       just iterate over the regions+indexes directly. instead of doing
				//       chunkToRegion + chunkToRegionIndex for every chunk in the Range. It would
				//       also be more efficient because we would only need to do chunkToRegion
				//       once and then can use offsets instead of per chunk.
				for (auto chunkCoord = area.min; chunkCoord.x < area.max.x; ++chunkCoord.x) {
					for (chunkCoord.y = area.min.y; chunkCoord.y < area.max.y; ++chunkCoord.y) {
						const auto regionCoord = chunkToRegion(chunkCoord);
						reserve(regionCoord);

						auto& regionStore = at(regionCoord);
						const auto regionIndex = chunkToRegionIndex(chunkCoord, regionCoord);

						if (!regionStore.isPopulated(regionIndex)) {
							regionStore.setPopulated(regionIndex);
							auto& chunkData = regionStore.at(regionIndex);
							func(chunkCoord, chunkData);
						}
					}
				}
			}
	};
}
