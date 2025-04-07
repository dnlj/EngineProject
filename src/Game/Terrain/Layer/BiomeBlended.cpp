// Game
#include <Game/Terrain/Layer/BiomeBlended.hpp>
#include <Game/Terrain/Layer/BiomeBlended.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>
#include <Game/Terrain/biomes/all.hpp>


namespace Game::Terrain::Layer {
	void BiomeBlended::request(const Range area, TestGenerator& generator) {
		ENGINE_LOG2("BiomeBlended::request area=({}, {})", area.min, area.max);
		generator.request<BiomeWeights>(area);
	}

	void BiomeBlended::generate(const Range area, TestGenerator& generator) {
		ENGINE_LOG2("BiomeBlended::generate area=({}, {})", area.min, area.max);
		cache.forEachChunk(area, [&](ChunkVec chunkCoord, auto& chunkStore) ENGINE_INLINE {
			const auto& chunkBiomeWeights = generator.get<BiomeWeights>(chunkCoord);
			const auto baseBlockCoord = chunkToBlock(chunkCoord);
			for (BlockVec chunkIndex = {0, 0}; chunkIndex.x < chunkSize.x; ++chunkIndex.x) {
				for (chunkIndex.y = 0; chunkIndex.y < chunkSize.y; ++chunkIndex.y) {
					const auto blockCoord = baseBlockCoord + chunkIndex;
					chunkStore.at(chunkIndex) = populate(blockCoord, chunkBiomeWeights.at(chunkIndex), generator);
				}
			}
		});
	}

	const ChunkStore<BiomeBlend>& BiomeBlended::get(const Index chunkCoord) const noexcept {
		const auto regionCoord = chunkToRegion(chunkCoord);
		return cache.at(regionCoord).at(chunkToRegionIndex(chunkCoord, regionCoord));
	}

	[[nodiscard]] BiomeBlend BiomeBlended::populate(BlockVec blockCoord,  BiomeBlend blend, const TestGenerator& generator) const noexcept {
		normalizeBiomeWeights(blend.weights);
		blend.rawWeights = blend.weights;
		ENGINE_DEBUG_ONLY({
			const auto normTotal = std::reduce(blend.weights.cbegin(), blend.weights.cend(), 0.0f, [](Float accum, const auto& value){ return accum + value.weight; });
			ENGINE_DEBUG_ASSERT(std::abs(1.0f - normTotal) <= FLT_EPSILON, "Incorrect normalized biome weight total: ", normTotal);
		});

		// TODO: Does each biome need to specify its own basis strength? Again, this is
		//       the _strength_ of the basis, not the basis itself. I don't think they do.
		//       We could probably just create sizeof(Biomes) simplex samplers and then
		//       use the BiomeId+1/2/3 for the weight of any given biome. Its fine being
		//       defined on the biomes for now though until we have more complete use
		//       cases.

		for (auto& biomeWeight : blend.weights) {
			// Output should be between 0 and 1. This is the strength of the basis, not the basis itself.
			const auto basisStr = generator.rm_getBasisStrength(biomeWeight.id, blockCoord);
			ENGINE_DEBUG_ASSERT(0.0f <= basisStr && basisStr <= 1.0f, "Invalid basis strength value given for biome ", biomeWeight.id, ". Out of range [0, 1].");

			// Multiply with the existing weight to get a smooth transition.
			ENGINE_DEBUG_ASSERT(0.0f <= biomeWeight.weight && biomeWeight.weight <= 1.0f, "Invalid biome blend weight for biome ", biomeWeight.id, ". Out of range [0, 1].");
			biomeWeight.weight *= basisStr;
		}

		return blend;
	}
}
