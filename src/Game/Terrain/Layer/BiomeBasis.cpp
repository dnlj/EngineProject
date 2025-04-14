/*
// Game
#include <Game/Terrain/Layer/BiomeBasis.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>
#include <Game/Terrain/biomes/all.hpp>


namespace Game::Terrain::Layer {
	void BiomeBasis::request(const Range area, TestGenerator& generator) {
		generator.request<BiomeBlended>(area);
	}

	void BiomeBasis::generate(const Range area, TestGenerator& generator) {
		// TODO: consider if an iterator approach would be better, like we do in BiomeHeight.
		//cache.forEachChunk(area, [&](const ChunkVec chunkCoord, auto& chunkStore) ENGINE_INLINE_REL {
		//	const auto& store = generator.get<BiomeBlended>(chunkCoord);
		//	const auto baseBlockCoord = chunkToBlock(chunkCoord);
		//	for (BlockVec chunkIndex = {0, 0}; chunkIndex.x < chunkSize.x; ++chunkIndex.x) {
		//		for (chunkIndex.y = 0; chunkIndex.y < chunkSize.y; ++chunkIndex.y) {
		//			const auto blockCoord = baseBlockCoord + chunkIndex;
		//			chunkStore.at(chunkIndex) = populate(blockCoord, chunkBiomeWeights.at(chunkIndex), generator);
		//		}
		//	}
		//});
	}

	//const ChunkStore<BasisInfo>& BiomeBasis::get(const Index chunkCoord) const noexcept {
	//}

	// TODO: Why does the basis info need so much data? Can we simplify it with the Layer
	//       architecture? I think so. Looks like most the other data isn't even used. And
	//       what is is only in the preview.
	BasisInfo BiomeBasis::populate(BlockVec blockCoord, const BiomeBlend& blend, const TestGenerator& generator) const noexcept {
		//const auto blend = calcBiome(blockCoord, h0);
		//const Float h0F = static_cast<Float>(h0);
		//const auto h2 = static_cast<Float>(layerBiomeHeight.get(blockCoord.x));
		
		Float totalBasis = 0;
		for (auto& biomeWeight : blend.weights) {
			//BIOME_GEN_DISPATCH_REQUIRED(getBasis, Float, TERRAIN_GET_BASIS_ARGS);
			//const auto getBasis = BIOME_GET_DISPATCH(getBasis, biomeWeight.id);
			//const auto basis = getBasis(biomes, blockCoord, h0F, h2, blend.info, biomeWeight.weight);
			const auto basis = generator.rm_getBasis(biomeWeight.id, blockCoord);
		
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
*/
