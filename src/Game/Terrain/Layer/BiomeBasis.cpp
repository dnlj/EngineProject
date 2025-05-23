// Game
#include <Game/Terrain/Layer/BiomeBasis.hpp>
#include <Game/Terrain/Layer/BiomeHeight.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>


namespace Game::Terrain::Layer {
	void BiomeBasis::request(const Range area, TestGenerator& generator) {
		generator.request<BiomeBlended>(area);
		generator.request<BiomeHeight>(ChunkSpanX{area.min.x, area.max.x}.toRegionSpan());
	}

	void BiomeBasis::generate(const Range area, TestGenerator& generator) {
		// TODO: consider if an iterator approach would be better, like we do in BiomeHeight.
		cache.forEachChunk(area, [&](const ChunkVec chunkCoord, auto& basisStore) ENGINE_INLINE_REL {
			const auto& blendStore = generator.get<BiomeBlended>(chunkCoord);
			const auto baseBlockCoord = chunkToBlock(chunkCoord);
			for (BlockVec chunkIndex = {0, 0}; chunkIndex.x < chunkSize.x; ++chunkIndex.x) {
				for (chunkIndex.y = 0; chunkIndex.y < chunkSize.y; ++chunkIndex.y) {
					const auto blockCoord = baseBlockCoord + chunkIndex;
					basisStore.at(chunkIndex) = populate(blockCoord, blendStore.at(chunkIndex), generator);
				}
			}
		});
	}

	const ChunkStore<BasisInfo>& BiomeBasis::get(const Index chunkCoord) const noexcept {
		const auto regionCoord = chunkToRegion(chunkCoord);
		return cache.at(regionCoord).at(chunkToRegionIndex(chunkCoord, regionCoord));
	}

	BasisInfo BiomeBasis::populate(BlockVec blockCoord, const BiomeBlend& blend, const TestGenerator& generator) const noexcept {
		Float totalBasis = 0;
		const auto h2 = generator.get<BiomeHeight>(blockCoord.x);
		for (auto& biomeWeight : blend.weights) {
			// TODO: rm - const auto basis = generator.rm_getBasis(biomeWeight.id, blockCoord);
			const auto basis = Engine::withTypeAt<Biomes>(biomeWeight.id, [&]<class Biome>(){
				return generator.get2<typename Biome::Basis>(blockCoord, h2);
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
