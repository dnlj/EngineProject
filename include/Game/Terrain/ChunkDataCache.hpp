#pragma once


namespace Game::Terrain {
	/**
	 * Store data per block for each chunk.
	 */
	template<class BlockData>
	class ChunkDataCache : public RegionDataCache<ChunkStore<BlockData>> {
	};
}
