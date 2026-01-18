#pragma once

// Game
#include <Game/Terrain/Layer/BiomeFoo.hpp>
#include <Game/Terrain/Layer/WorldBaseHeight.hpp>

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
	void BiomeFooHeight::request(const Range<Partition>& regionCoordXs, TestGenerator& generator) {
		regionCoordXs.forEach([&](const Partition& regionCoordX){
			generator.request<WorldBaseHeight>(regionCoordX);
		});
	}

	Float BiomeFooHeight::get(BIOME_HEIGHT_ARGS) const noexcept {
		auto& simplex = generator.shared<BiomeFooSharedData>().simplex;
		return h0 + 15 * simplex.value(blockCoordX.pos * 0.05_f, 0); // TODO: 1d simplex
	}

	Float BiomeFooWeight::get(BIOME_WEIGHT_ARGS) const noexcept {
		auto const& simplex = generator.shared<BiomeFooSharedData>().simplex;
		return 0.5_f + 0.5_f * simplex.value(blockCoordF * 0.03_f);
	}

	Float BiomeFooBasis::get(BIOME_BASIS_ARGS) const noexcept {
		if (blockCoord.pos.y > h2) { return outGrad(static_cast<Float>(h2), blockCoord.pos.y, 1.0_f / 5.0_f); }

		// TODO: redo this, extract some helpers from the debug biomes.
		auto& simplex = generator.shared<BiomeFooSharedData>().simplex;
		constexpr Float scale = 0.06_f;
		constexpr Float groundScale = 1.0_f / 100.0_f;
		Float value =
			+ inGrad(h2, blockCoord.pos.y, groundScale)
			+ simplex.value(blockCoordF * scale);

		return std::clamp(value, -1_f, 1_f);
	}

	BlockId BiomeFooBlock::get(BIOME_BLOCK_ARGS) const noexcept {
		auto& simplex = generator.shared<BiomeFooSharedData>().simplex;

		if (blockCoord.pos.x % chunkSize.x == 0) { return BlockId::Debug1; }
		if (blockCoord.pos.y % chunkSize.y == 0) { return BlockId::Debug2; }

		if (blockCoord.pos.y > h2 - 3) {
			return BlockId::Grass;
		}

		struct ResourceSpec {
			consteval ResourceSpec(BlockId b, Float s, Float d)
				: block{b}, scale{1.0_f/s}, density{d * 2.0_f - 1.0_f} {
			}

			BlockId block = BlockId::None;
			Float scale = 1.0_f;
			Float density = 0.0_f;
		};

		// TODO: min/max depth range with taper distance.
		constexpr ResourceSpec ores[] = {
			{BlockId::Gold, 7.0_f, 0.11_f},
			{BlockId::Iron, 5.0_f, 0.2_f},
		};

		for (const auto& ore : ores) {
			if (simplex.value(blockCoordF * ore.scale) < ore.density) {
				return ore.block;
			}
		}

		return BlockId::Dirt;
	}

	void BiomeFooStructureInfo::get(BIOME_STRUCTURE_INFO_ARGS) const noexcept {
		const auto minBlockCoord = chunkCoord.toBlock().pos;

		constexpr BlockUnit width = 60;
		constexpr BlockUnit spacing = 2*width;
		constexpr BlockUnit stride = spacing + width;

		// TODO: Could be smart about iteration instead of checking every block += stride, etc.
		const auto maxBlockCoord = minBlockCoord + chunkSize;
		auto h2It = generator.get<BlendedBiomeHeight>(chunkCoord.toX());
		for (auto blockCoord = minBlockCoord; blockCoord.x < maxBlockCoord.x; ++blockCoord.x, ++h2It) {
			if (blockCoord.x % stride == 0) {
				blockCoord.y = *h2It;
				if (blockCoord.y >= minBlockCoord.y && blockCoord.y < maxBlockCoord.y) {
					inserter = {blockCoord, blockCoord + BlockVec{width, width}, 0};
				}
			}
		}
	}

	void BiomeFooStructure::get(BIOME_STRUCTURE_ARGS) const noexcept {
		// TODO: Come up with a system for landmark ids.
		//       Currently:
		//       - info.id = 0 = tree

		// TODO: This is the least efficient way possible to do this. We need a good
		//       api to efficiently edit multiple blocks spanning multiple chunks and
		//       regions. We would need to pre split between both regions and then chunks
		//       and then do something roughly like:
		//       for region in splitRegions:
		//           for chunk in splitRegionChunks:
		//               applyEdit(chunk, editsForChunk(chunk));
		for (UniversalBlockCoord blockCoord = {realmId, info.min}; blockCoord.pos.x < info.max.x; ++blockCoord.pos.x) {
			for (blockCoord.pos.y = info.min.y; blockCoord.pos.y < info.max.y; ++blockCoord.pos.y) {
				const UniversalChunkCoord chunkCoord = blockCoord.toChunk();
				const UniversalRegionCoord regionCoord = chunkCoord.toRegion();
				auto& region = terrain.getRegion(regionCoord);
				const auto regionIdx = chunkCoord.toRegionIndex(regionCoord);
				auto& chunk = region.chunks[regionIdx.x][regionIdx.y];
				const auto chunkIdx = blockCoord.toChunkIndex(chunkCoord);
				ENGINE_DEBUG_ASSERT(chunkIdx.x >= 0 && chunkIdx.x < chunkSize.x);
				ENGINE_DEBUG_ASSERT(chunkIdx.y >= 0 && chunkIdx.y < chunkSize.y);
				ENGINE_DEBUG_ASSERT(region.getChunkStage(regionIdx) == ChunkStage::TerrainComplete);

				chunk.data[chunkIdx.x][chunkIdx.y] = BlockId::Gold;
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

