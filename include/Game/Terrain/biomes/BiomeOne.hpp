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
			
			constexpr Float scale = 0.06_f;
			constexpr Float groundScale = 1.0_f / 100.0_f;
			return
				+ inGrad(h1, blockCoord.y, groundScale)
				+ simplex.value(glm::vec2{blockCoord} * scale);
		}

		void getLandmarks(TERRAIN_GET_LANDMARKS_ARGS) {
			//ENGINE_LOG2("GET LANDMARK: {}", chunkCoord);
			auto blockCoord = chunkToBlock(chunkCoord);
			inserter = {.min = blockCoord, .max = blockCoord, .id = 1};

			const auto maxX = blockCoord.x + chunkSize.x;
			for (; blockCoord.x < maxX; ++blockCoord.x) {
				if (blockCoord.x % 7 == 0) {
					if (blockCoord.y >= heightCache.get(blockCoord.x))
					{
						// TODO: land on surface

						inserter = {blockCoord, blockCoord + BlockVec{2, 8}, 0};
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
