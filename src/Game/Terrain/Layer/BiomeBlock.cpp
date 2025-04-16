// Game
#include <Game/Terrain/Layer/BiomeBlock.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>
#include <Game/Terrain/biomes/all.hpp>


namespace Game::Terrain::Layer {
	void BiomeBlock::request(const Range area, TestGenerator& generator) {
		generator.request<BiomeBasis>(area);
	}

	void BiomeBlock::generate(const Range area, TestGenerator& generator) {
		cache.forEachChunk(area, [&](const ChunkVec chunkCoord, MapChunk& chunkStore) ENGINE_INLINE_REL {
			const auto& chunkBiomeBasis = generator.get<BiomeBasis>(chunkCoord);
			const auto baseBlockCoord = chunkToBlock(chunkCoord);
			for (BlockVec chunkIndex = {0, 0}; chunkIndex.x < chunkSize.x; ++chunkIndex.x) {
				for (chunkIndex.y = 0; chunkIndex.y < chunkSize.y; ++chunkIndex.y) {
					const auto blockCoord = baseBlockCoord + chunkIndex;
					const auto& biomeBasis = chunkBiomeBasis.at(chunkIndex);
					chunkStore.data[chunkIndex.x][chunkIndex.y] = populate(blockCoord, biomeBasis, generator);
				}
			}
		});
	}

	const MapChunk& BiomeBlock::get(const Index chunkCoord) const noexcept {
		const auto regionCoord = chunkToRegion(chunkCoord);
		return cache.at(regionCoord).at(chunkToRegionIndex(chunkCoord, regionCoord));
	}

	[[nodiscard]] BlockId BiomeBlock::populate(const BlockVec blockCoord, const BasisInfo& basisInfo, const TestGenerator& generator) const noexcept {
		if (basisInfo.basis <= 0.0_f) {
			// TODO: is there a reason we don't just default to air as 0/{}? Do we need BlockId::None?
			return BlockId::Air;
		}

		return generator.rm_getStage(basisInfo.id, blockCoord, basisInfo);
	}
}
