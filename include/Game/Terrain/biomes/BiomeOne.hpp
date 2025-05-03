#include <Game/Terrain/biome.hpp>


// TODO: Remove if never needed. This is to test vertically if an area is open.
/*const auto testY = [&](BlockVec blockCoord){
	// TODO: should probably use distance (meters) and translate to blocks instead of blocks directly since block scale will change.
	constexpr static BlockUnit queryD = 10;

	// TODO: its not 100% that we can use these right? they assume the start coord is in the query? thats probably fine?
	auto curRegionIdx = regionIdx;
	auto curRegionCoord = regionCoord;
	Region const* curRegion = &terrain.getRegion(curRegionCoord); // TODO: should pass in initially
	//Chunk const* curChunk = &chunk; // TODO: its not 100% that the test coord is the same as the landmak chunk right?
	Chunk const* curChunk = &curRegion->chunkAt(curRegionIdx);
	auto rem = queryD;
	const auto initialChunkIdx = blockToChunkIndex(blockCoord, chunkCoord);
	const BlockUnit x = initialChunkIdx.x;
	BlockUnit y = initialChunkIdx.y;

	while (true) {
		while (true) {
			while (y < chunkSize.y) {
				if (chunk.data[x][y] != BlockId::Air) {
					// Found a collision, abort early.
					return false;
				}

				++y;
				--rem;
			
				if (rem == 0) {
					// Search done, no collision found.
					return true;
				}
			}

			y = 0;
			++curRegionIdx.y;
			if (curRegionIdx.y >= regionSize.y) { break; }
			curChunk = &curRegion->chunkAt(curRegionIdx);
		}

		curRegionIdx.y = 0;
		++curRegionCoord.pos.y;
		curRegion = &terrain.getRegion(curRegionCoord);
	}
};*/

namespace Game::Terrain {
	struct BiomeOne : public SimpleBiome {
		Engine::Noise::OpenSimplexNoise simplex{1234};

		void getLandmarks(TERRAIN_GET_LANDMARKS_ARGS) const {
			//ENGINE_LOG2("GET LANDMARK: {}", chunkCoord);
			const auto minBlockCoord = chunkToBlock(chunkCoord);
			inserter = {.min = minBlockCoord, .max = minBlockCoord + BlockVec{1,1}, .id = 1};

			constexpr BlockUnit width = 3;
			constexpr BlockUnit spacing = 9;
			constexpr BlockUnit stride = spacing + width;

			// TODO: coudl step more that ++1 since we know we have a fixed modulus.
			const auto maxBlockCoord = minBlockCoord + chunkSize;
			for (auto blockCoord = minBlockCoord; blockCoord.x < maxBlockCoord.x; ++blockCoord.x) {
				if (blockCoord.x % stride == 0) {
					blockCoord.y = h2Cache.get(blockCoord.x);
					if (blockCoord.y >= minBlockCoord.y && blockCoord.y < maxBlockCoord.y) {
						// TODO: random horizontal variation, etc.
						inserter = {blockCoord, blockCoord + BlockVec{width, 12}, 0};
					}
				}
			}
		}

		void genLandmarks(TERRAIN_GEN_LANDMARKS_ARGS) const {
			// TODO: Come up with a system for landmark ids.
			//       Currently:
			//       - info.id = 0 = tree
			//       - info.id = 1 = debug chunk marker

			// TODO: This is the least efficient way possible to do this. We need a good
			//       api to efficiently edit multiple blocks spanning multiple chunks and
			//       regions. We would need to pre split between both regions and then chunks
			//       and then do something roughly like:
			//       for region in splitRegions:
			//           for chunk in splitRegionChunks:
			//               applyEdit(chunk, editsForChunk(chunk));
			for (auto blockCoord = info.min; blockCoord.x < info.max.x; ++blockCoord.x) {
				for (blockCoord.y = info.min.y; blockCoord.y < info.max.y; ++blockCoord.y) {
					const auto chunkCoord = blockToChunk(blockCoord);
					const UniversalRegionCoord regionCoord = {realmId, chunkToRegion(chunkCoord)};
					auto& region = terrain.getRegion(regionCoord);
					const auto regionIdx = chunkToRegionIndex(chunkCoord);
					auto& chunk = region.chunks[regionIdx.x][regionIdx.y];
					const auto chunkIdx = blockToChunkIndex(blockCoord, chunkCoord);
					ENGINE_DEBUG_ASSERT(chunkIdx.x >= 0 && chunkIdx.x < chunkSize.x);
					ENGINE_DEBUG_ASSERT(chunkIdx.y >= 0 && chunkIdx.y < chunkSize.y);

					// TODO: what is this debug4 check for?
					if (chunk.data[chunkIdx.x][chunkIdx.y] != BlockId::Debug4) {
						chunk.data[chunkIdx.x][chunkIdx.y] = info.id == 0 ? BlockId::Gold : BlockId::Grass;
					}

					//ENGINE_LOG2("GEN LANDMARK: {}", chunkCoord);
				}
			}

			if (info.id == 0) {
				const auto chunkCoord = blockToChunk(info.min);
				const UniversalRegionCoord regionCoord = {realmId, chunkToRegion(chunkCoord)};
				auto& region = terrain.getRegion(regionCoord);
				const auto regionIdx = chunkToRegionIndex(chunkCoord);
				auto& ents = region.entitiesAt(regionIdx);
				auto& ent = ents.emplace_back();
				ent.pos = info.min; // TODO: center
				ent.data.type = BlockEntityType::Tree;
				ent.data.asTree.type = Engine::Noise::lcg(ent.pos.x) % 3; // TODO: random tree variant
				ent.data.asTree.size = {3, 9}; // TODO: size
			}
		}
	};
}
