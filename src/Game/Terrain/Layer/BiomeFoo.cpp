#pragma once

// Game
#include <Game/Terrain/Layer/BiomeFoo.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>


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

// TODO: move to layer utils or similar.
namespace Game::Terrain::Layer {
	namespace
	{
		// TODO: doc, blends from 1 to 0
		ENGINE_INLINE constexpr Float inGrad(Float h, BlockUnit y, Float scale) noexcept {
			return std::max(0.0_f, 1.0_f - (h - y) * scale);
		}

		ENGINE_INLINE constexpr Float inGrad(BlockUnit h, BlockUnit y, Float scale) noexcept {
			// This is fine since h will always be within float range of 0.
			return inGrad(static_cast<Float>(h), y, scale);
		}
			
		// TODO: doc, blends from 0 to -1
		ENGINE_INLINE constexpr Float outGrad(Float h, BlockUnit y, Float scale) noexcept {
			return  std::max(-1.0_f, (h - y) * scale);
		}
	}
}

namespace Game::Terrain::Layer {
	void BiomeFooHeight::request(const Range area, TestGenerator& generator) {
		generator.request<WorldBaseHeight>(area.toRegionSpan());
	}

	Float BiomeFooHeight::get(BIOME_HEIGHT_ARGS) const noexcept {
		auto& simplex = generator.shared<BiomeFooSharedData>().simplex;
		return h0 + 15 * simplex.value(blockCoordX * 0.05_f, 0); // TODO: 1d simplex
	}

	void BiomeFooBasisStrength::request(const Range area, TestGenerator& generator) {
		// TODO: request verifier
	}

	Float BiomeFooBasisStrength::get(BIOME_BASIS_STRENGTH_ARGS) const noexcept {
		auto const& simplex = generator.shared<BiomeFooSharedData>().simplex;
		return 0.5_f + 0.5_f * simplex.value(glm::vec2{blockCoord} * 0.03_f);
	}

	Float BiomeFooBasis::get(BIOME_BASIS_ARGS) const noexcept {
		if (blockCoord.y > h2) { return outGrad(static_cast<Float>(h2), blockCoord.y, 1.0_f / 5.0_f); }

		// TODO: redo this, extract some helpers from the debug biomes.
		auto& simplex = generator.shared<BiomeFooSharedData>().simplex;
		constexpr Float scale = 0.06_f;
		constexpr Float groundScale = 1.0_f / 100.0_f;
		Float value =
			+ inGrad(h2, blockCoord.y, groundScale)
			+ simplex.value(glm::vec2{blockCoord} * scale);

		return std::clamp(value, -1_f, 1_f);
	}

	// TODO: BiomeFooBlock
	//STAGE(1) {
	//	// TODO: if we are always going to be converting to float anyways, should we
	//	//       pass in a float version as well? That kind of breaks world size though.
	//
	//	//
	//	//
	//	//
	//	//
	//	//
	//	//
	//	// TODO: change to use basis
	//	//
	//	//
	//	//
	//	//
	//	//
	//	//
	//	//
	//	//
	//	//
	//	//
	//	//
	//
	//
	//	// if y > h0 && blocksEmpty(y, y+5);
	//
	//	//const auto h1 = h0 + 15 * simplex.value(blockCoord.x * 0.05_f, 0); // TODO: 1d simplex
	//	//
	//	//if (blockCoord.y > h1) {
	//	//	return BlockId::Air;
	//	//} else if ((h1-blockCoord.y) < 1) {
	//	//	return BlockId::Grass;
	//	//}
	//
	//	//constexpr Float scale = 0.06_f;
	//	//constexpr Float groundScale = 1.0_f / 100.0_f;
	//	//const Float groundGrad = std::max(0.0_f, 1.0_f - (h1 - blockCoord.y) * groundScale);
	//	//const auto val = simplex.value(glm::vec2{blockCoord} * scale) + groundGrad;
	//	//
	//	//if (val > 0) {
	//	//	return BlockId::Debug;
	//	//} else {
	//	//	return BlockId::Air;
	//	//}
	//
	//	return BlockId::Debug;
	//}
	
	void BiomeFooStructureInfo::get(BIOME_STRUCTURE_INFO_ARGS) const noexcept {
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
				blockCoord.y = generator.get<BiomeHeight>(blockCoord.x);
				if (blockCoord.y >= minBlockCoord.y && blockCoord.y < maxBlockCoord.y) {
					// TODO: random horizontal variation, etc.
					inserter = {blockCoord, blockCoord + BlockVec{width, 12}, 0};
				}
			}
		}
	}

	void BiomeFooStructure::get(BIOME_STRUCTURE_ARGS) const noexcept {
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
}

