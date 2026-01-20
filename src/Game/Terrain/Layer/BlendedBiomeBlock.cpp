// Game
#include <Game/Terrain/Layer/BlendedBiomeBlock.hpp>
#include <Game/Terrain/Layer/BlendedBiomeBasis.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>


namespace Game::Terrain::Layer {
	void BlendedBiomeBlock::request(const Range<Partition>& chunkCoords, TestGenerator& generator) {
		// TODO: We should technically be making request calls to the relevant biomes for
		//       chunkCoord here. Also in other Blended* systems?
		chunkCoords.forEach([&](const Partition& chunkCoord){
			generator.request<BlendedBiomeBasis>(chunkCoord);
			cache.reserve(chunkCoord.toRegion(), getSeq());
		});
	}

	void BlendedBiomeBlock::generate(const Partition chunkCoord, TestGenerator& generator) {
		cache.populate(chunkCoord, getSeq(), [&](MapChunk& chunkStore) ENGINE_INLINE_REL {
			const auto& chunkBiomeBasis = generator.get<BlendedBiomeBasis>(chunkCoord);
			const auto& chunkHeight = generator.get<BlendedBiomeHeight>(chunkCoord.toX());
			const auto baseBlockCoord = chunkCoord.toBlock();
			for (BlockVec chunkIndex = {0, 0}; chunkIndex.x < chunkSize.x; ++chunkIndex.x) {
				const auto h2 = chunkHeight[chunkIndex.x];
				for (chunkIndex.y = 0; chunkIndex.y < chunkSize.y; ++chunkIndex.y) {
					const auto blockCoord = baseBlockCoord + chunkIndex;
					const auto& biomeBasis = chunkBiomeBasis.at(chunkIndex);
					chunkStore.data[chunkIndex.x][chunkIndex.y] = populate(blockCoord, h2, biomeBasis, generator);
				}
			}
		});
	}

	const MapChunk& BlendedBiomeBlock::get(const Index chunkCoord) const noexcept {
		const auto regionCoord = chunkCoord.toRegion();
		return cache.at(regionCoord, getSeq()).at(chunkCoord.toRegionIndex(regionCoord));
	}

	[[nodiscard]] BlockId BlendedBiomeBlock::populate(const UniversalBlockCoord blockCoord, const BlockUnit h2, const BasisInfo& basisInfo, const TestGenerator& generator) const noexcept {
		if (basisInfo.basis <= 0.0_f) {
			return BlockId::Air;
		}

		const FVec2 blockCoordF = blockCoord.pos;
		return Engine::withTypeAt<Biomes>(basisInfo.id, [&]<class Biome>(){
			return generator.get2<typename Biome::Block>(blockCoord, blockCoordF, h2, basisInfo);
		});
	}
}
