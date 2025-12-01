#pragma once

// Game
#include <Game/Terrain/ChunkStore.hpp>
#include <Game/Terrain/RegionDataCache.hpp>


namespace Game::Terrain {
	/**
	 * Store data per block for each chunk.
	 */
	template<class BlockData>
	class ChunkDataCache : public RegionDataCache<ChunkStore<BlockData>> {
	};

	template<class T>
	ENGINE_INLINE inline void removeGeneratedPartitions(ChunkDataCache<T>& cache, SeqNum seqNum, std::vector<ChunkVec>& partitions) {
		std::erase_if(partitions, [&](const ChunkVec& chunkCoord){ return cache.isPopulated(chunkCoord, seqNum); });
	}
}
