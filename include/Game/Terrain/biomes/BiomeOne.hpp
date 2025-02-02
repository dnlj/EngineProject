#include <Game/Terrain/biome.hpp>

namespace Game::Terrain {
	struct BiomeOne : public SimpleBiome {
		STAGE_DEF;

		Engine::Noise::OpenSimplexNoise simplex{1234};

		STAGE(1) {
			// TODO: if we are always going to be converting to float anyways, should we
			//       pass in a float version as well? That kind of breaks world size though.

			//
			//
			//
			//
			//
			//
			// TODO: change to use basis
			//
			//
			//
			//
			//
			//
			//
			//
			//
			//
			//


			// if y > h0 && blocksEmpty(y, y+5);

			//const auto h1 = h0 + 15 * simplex.value(blockCoord.x * 0.05_f, 0); // TODO: 1d simplex
			//
			//if (blockCoord.y > h1) {
			//	return BlockId::Air;
			//} else if ((h1-blockCoord.y) < 1) {
			//	return BlockId::Grass;
			//}

			//constexpr Float scale = 0.06_f;
			//constexpr Float groundScale = 1.0_f / 100.0_f;
			//const Float groundGrad = std::max(0.0_f, 1.0_f - (h1 - blockCoord.y) * groundScale);
			//const auto val = simplex.value(glm::vec2{blockCoord} * scale) + groundGrad;
			//
			//if (val > 0) {
			//	return BlockId::Debug;
			//} else {
			//	return BlockId::Air;
			//}

			return BlockId::Debug;
		}

		//STAGE(2) {
		//	auto& blockId = chunk.data[blockIndex.x][blockIndex.y];
		//	if (((blockCoord.x & 1) ^ (blockCoord.y & 1)) && (blockId == BlockId::Air)) {
		//		return BlockId::Debug2;
		//	} else {
		//		return blockId;
		//	}
		//}

		Float getBasisStrength(TERRAIN_GET_BASIS_STRENGTH_ARGS) {
			return 0.5_f + 0.5_f * simplex.value(glm::vec2{blockCoord} * 0.03_f);
		}

		Float getBasis(TERRAIN_GET_BASIS_ARGS) {
			const auto h1 = h0 + 15 * simplex.value(blockCoord.x * 0.05_f, 0); // TODO: 1d simplex
			if (blockCoord.y > h1) { return outGrad(h1, blockCoord.y, 1.0_f / 5.0_f); }

			// TODO: redo this, extract some helpers from the debug biomes.
			constexpr Float scale = 0.06_f;
			constexpr Float groundScale = 1.0_f / 100.0_f;
			Float value =
				+ inGrad(h1, blockCoord.y, groundScale)
				+ simplex.value(glm::vec2{blockCoord} * scale);

			return std::clamp(value, -1_f, 1_f);
		}

		void getLandmarks(TERRAIN_GET_LANDMARKS_ARGS) {
			//ENGINE_LOG2("GET LANDMARK: {}", chunkCoord);
			auto blockCoord = chunkToBlock(chunkCoord);
			inserter = {.min = blockCoord, .max = blockCoord, .id = 1};


			// TODO: need to cull based on h2 instead.
			{ // Cull any chunks outside of the chunk containing the ground level. This won't work since this is h0 not h1.
				const auto h0 = h0Cache.get(blockCoord.x);
				if (blockCoord.y < h0 || blockCoord.y > h0 + chunkSize.y) {
					return;
				}
			}

			//
			//
			//
			//
			//
			//
			//
			// TODO: If we now have h2 do we even need the y test?
			// - add h2 cache
			//
			//
			//
			//
			//
			//
			//
			//
			//
			//

			const auto testY = [&](BlockVec blockCoord){
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
			};

			const auto maxX = blockCoord.x + chunkSize.x;
			for (; blockCoord.x < maxX; ++blockCoord.x) {
				if (blockCoord.x % 7 == 0) {
					if (blockCoord.y >= h0Cache.get(blockCoord.x))
					{
						// TODO: land on surface

						if (testY(blockCoord)) {
							inserter = {blockCoord, blockCoord + BlockVec{2, 8}, 0};
						}
					}
				}
			}
		}

		void genLandmarks(TERRAIN_GEN_LANDMARKS_ARGS) {
			// TODO: This is the least efficient way possible to do this. We need a good
			//       api to efficiently edit multiple blocks spanning multiple chunks and
			//       regions. We would need to pre split between both regions and then chunks
			//       and then do something roughly like:
			//       for region in splitRegions:
			//           for chunk in splitRegionChunks:
			//               applyEdit(chunk, editsForChunk(chunk));
			for (auto blockCoord = info.min; blockCoord.x <= info.max.x; ++blockCoord.x) {
				for (blockCoord.y = info.min.y; blockCoord.y <= info.max.y; ++blockCoord.y) {
					const auto chunkCoord = blockToChunk(blockCoord);
					const UniversalRegionCoord regionCoord = {info.realmId, chunkToRegion(chunkCoord)};
					auto& region = terrain.getRegion(regionCoord);
					const auto regionIdx = chunkToRegionIndex(chunkCoord);
					auto& chunk = region.chunks[regionIdx.x][regionIdx.y];
					const auto chunkIdx = blockToChunkIndex(blockCoord, chunkCoord);
					ENGINE_DEBUG_ASSERT(chunkIdx.x >= 0 && chunkIdx.x < chunkSize.x);
					ENGINE_DEBUG_ASSERT(chunkIdx.y >= 0 && chunkIdx.y < chunkSize.y);

					if (chunk.data[chunkIdx.x][chunkIdx.y] != BlockId::Debug4)
					{
						chunk.data[chunkIdx.x][chunkIdx.y] = info.id == 0 ? BlockId::Gold : BlockId::Debug3;
					}
					//ENGINE_LOG2("GEN LANDMARK: {}", chunkCoord);
				}
			}
		}
	};
}
