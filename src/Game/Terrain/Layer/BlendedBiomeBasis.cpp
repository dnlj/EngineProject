// Game
#include <Game/Terrain/Layer/BlendedBiomeBasis.hpp>
#include <Game/Terrain/Layer/BlendedBiomeHeight.hpp>
#include <Game/Terrain/Layer/BlendedBiomeWeights.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>


namespace Game::Terrain::Layer {
	void BlendedBiomeBasis::request(const Range<Partition>& chunkCoords, TestGenerator& generator) {
		chunkCoords.forEach([&](const Partition& chunkCoord){
			const auto regionCoord = chunkCoord.toRegion();
			generator.request<BlendedBiomeWeights>(chunkCoord);
			generator.request<BlendedBiomeHeight>(regionCoord.toX());
			cache.reserve(regionCoord, getSeq());
		});
	}

	void BlendedBiomeBasis::generate(const Partition chunkCoord, TestGenerator& generator) {
		cache.populate(chunkCoord, getSeq(), [&](auto& basisStore) ENGINE_INLINE_REL {
			const auto& blendStore = generator.get<BlendedBiomeWeights>(chunkCoord);
			const auto baseBlockCoord = chunkCoord.toBlock();
			auto h2It = generator.get<BlendedBiomeHeight>(chunkCoord.toX());
			for (BlockVec chunkIndex = {0, 0}; chunkIndex.x < chunkSize.x; ++chunkIndex.x, ++h2It) {
				const auto blockCoordX = baseBlockCoord.pos.x + chunkIndex.x;
				for (chunkIndex.y = 0; chunkIndex.y < chunkSize.y; ++chunkIndex.y) {
					const auto blockCoordY = baseBlockCoord.pos.y + chunkIndex.y;
					const UniversalBlockCoord blockCoord = {.realmId = baseBlockCoord.realmId, .pos = {blockCoordX, blockCoordY}};
					basisStore.at(chunkIndex) = populate(blockCoord, *h2It, blendStore.at(chunkIndex), generator);
				}
			}
		});
	}

	const ChunkStore<BasisInfo>& BlendedBiomeBasis::get(const Index chunkCoord) const noexcept {
		const auto regionCoord = chunkCoord.toRegion();
		return cache.at(regionCoord, getSeq()).at(chunkCoord.toRegionIndex(regionCoord));
	}

	BasisInfo BlendedBiomeBasis::populate(const UniversalBlockCoord blockCoord, const BlockUnit h2, const BiomeBlend& blend, const TestGenerator& generator) const noexcept {
		Float totalBasis = 0;
		FVec2 blockCoordF = blockCoord.pos;
		for (auto& biomeWeight : blend.weights) {
			const auto basis = Engine::withTypeAt<Biomes>(biomeWeight.id, [&]<class Biome>(){
				return generator.get2<typename Biome::Basis>(blockCoord, blockCoordF, h2);
			});

			// This is a _somewhat_ artificial limitation. A basis doesn't _need_ to be
			// between [-1, 1], but all biomes should have roughly the same range. If one
			// is [-100, 100] and another is [-1, 1] they won't blend well since the one
			// with the larger range will always dominate regardless of the blend weight.
			// Keeping things normalized avoids that.
			ENGINE_DEBUG_ASSERT(-1.0_f <= basis && basis <= 1.0_f, "Invalid basis value given for biome ", biomeWeight.id, ". Out of range [-1, 1].");
			totalBasis += biomeWeight.weight * basis;
		}

		const auto maxBiome = maxBiomeWeight(blend.weights);
		return {
			.id = maxBiome.id,
			.weight = maxBiome.weight,
			.basis = totalBasis,
		};
	}
}
