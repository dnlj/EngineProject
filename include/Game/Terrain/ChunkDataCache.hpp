#pragma once


namespace Game::Terrain {
	/**
	 * Store data per block for each chunk.
	 */
	template<class BlockData>
	class ChunkDataCache : public RegionDataCache<ChunkStore<BlockData>> {
		public:
			// TODO: just `at(chunkCoord)` once we have distinct defs for block/chunk/region types.
			ENGINE_INLINE ChunkStore<BlockData>& populateChunk(const auto chunkCoord) {
				const auto regionCoord = chunkToRegion(chunkCoord);
				auto& regionStore = this->at(regionCoord);

				const auto regionIndex = chunkToRegionIndex(chunkCoord, regionCoord);
				regionStore.setPopulated(regionIndex);
				return regionStore.at(regionIndex);
			}

			// TODO (dBWaSv4T): remove once we have generator level populated checks.
			ENGINE_INLINE bool isPopulated(const auto chunkCoord) const {
				const auto regionCoord = chunkToRegion(chunkCoord);
				const auto& regionStore = this->at(regionCoord);
				const auto regionIndex = chunkToRegionIndex(chunkCoord, regionCoord);
				return regionStore.isPopulated(regionIndex);
			}
	};
}
