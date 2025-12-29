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
}
