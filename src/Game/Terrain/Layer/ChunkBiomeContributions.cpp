/*
// Game
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Layer/ChunkBiomeContributions.hpp>
#include <Game/Terrain/Layer/BlendedBiomeWeights.hpp>

namespace Game::Terrain::Layer {
	void ChunkBiomeContributions::request(const Partition chunkCoord, TestGenerator& generator) {
		const auto regionCoord = chunkCoord.toRegion();
		generator.request<BlendedBiomeWeights>(chunkCoord);
		cache.reserveRegion(regionCoord, getSeq());
	}

	void ChunkBiomeContributions::generate(const Partition chunkCoord, TestGenerator& generator) {
		// Get a unique list of the biome in each corner of this chunk. Used for during generation.
		// At generation time checking each corner should yield a unique list of all biomes in this
		// chunk so long as the minimum biome size is greater than or equal to the chunk size. This
		// four biome limit is due to the RawBiome layer which distributes biomes on a grid. Its not
		// pure random.

		cache.populate(chunkCoord, getSeq(), [&](BiomeContributions& biomes){
			const auto& chunkBlendedBiomeWeights = generator.get<BlendedBiomeWeights>(chunkCoord);
			const auto tryAdd = [&](const BlockVec chunkIndex) ENGINE_INLINE {
				const auto biomeId = maxBiomeWeight(chunkBlendedBiomeWeights.at(chunkIndex).weights).id;
				if (!std::ranges::contains(biomes, biomeId))  { biomes.push_back(biomeId); }
			};

			// We need -1 to chunkSize since these are indexes, not sizes.
			tryAdd({0, 0});
			tryAdd({0, chunkSize.y - 1});
			tryAdd({chunkSize.x - 1, 0});
			tryAdd({chunkSize.x - 1, chunkSize.y - 1});
		});

	}

	auto ChunkBiomeContributions::get(const Index chunkCoord) const noexcept -> const BiomeContributions& {
		const auto regionCoord = chunkCoord.toRegion();
		const auto& regionData = cache.at(regionCoord, getSeq());
		const auto regionIndex = chunkCoord.toRegionIndex(regionCoord);
		const auto& chunkBiomes = regionData.at(regionIndex);
		return chunkBiomes;
	}
}
*/
