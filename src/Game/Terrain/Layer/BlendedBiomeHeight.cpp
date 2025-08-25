// Game
#include <Game/Terrain/Layer/BlendedBiomeHeight.hpp>
#include <Game/Terrain/Layer/BlendedBiomeWeights.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>


namespace Game::Terrain::Layer {
	void BlendedBiomeHeight::request(const Range area, TestGenerator& generator) {
		cache.reserve(area);

		generator.requestAwait<WorldBaseHeight>(area);

		auto h0walk = generator.get<WorldBaseHeight>(area);
		{
			const auto blockMinX = area.min * chunksPerRegion * blocksPerChunk;
			const auto blockMaxX = area.max * chunksPerRegion * blocksPerChunk;
			auto hMin = *h0walk;
			auto hMax = hMin;
			for (++h0walk; h0walk; ++h0walk) {
				const auto h0 = *h0walk;
				hMin = std::min(hMin, h0);
				hMax = std::max(hMax, h0);
			}

			generator.request<BlendedBiomeWeights>({
				.min = blockToChunk({blockMinX, hMin}),
				.max = blockToChunk({blockMaxX, hMax}) + ChunkVec{0, 1}, // Add one to get an _exlusive_ bound instead of inclusive.
			});

			// TODO: We should probably be doing this with correct `request` calls to the blended biomes.
			// 
			// Note that we don't send requests to the biome in the blend. In theory we
			// should be doing a requestAwait for BlendedBiomeWeights and then also sending
			// requests to each biome in the resulting blend. Currently this is "okay" to
			// skip because all biome layers are expected to resolve in `get` anyways.
			// Although this does bypass the request/generate dependency model.
		}

	}

	void BlendedBiomeHeight::generate(const Partition regionCoordX, TestGenerator& generator) {
		// TODO: Add static asserts to ensure that the biome height layers _ARE NOT_
		//       cached. Since we don't issue requests cached height layers won't work. They
		//       also don't make sense. The BlendedBiomeHeight layer (this layer) is what should be
		//       doing the caching. Other layers should be sampling the BlendedBiomeHeight layer, not specific biomes.
		
		const auto& h0Data = generator.get3<WorldBaseHeight>(regionCoordX);
		auto& h2Data = cache.get(regionCoordX);
		const auto baseBlockCoordX = chunkToBlock(regionToChunk({regionCoordX, 0})).x;
		for (BlockUnit blockRegionIndex = 0; blockRegionIndex < blocksPerRegion; ++blockRegionIndex) {
			const auto blockCoordX = baseBlockCoordX + blockRegionIndex;
			const auto h0 = h0Data[blockRegionIndex];
			const auto h0F = static_cast<Float>(h0);
			const auto chunkCoord = blockToChunk({blockCoordX, h0});
			const auto& chunkStore = generator.get<BlendedBiomeWeights>(chunkCoord);
			const auto chunkIndex = blockToChunkIndex({blockCoordX, h0}, chunkCoord);
			const auto& blend = chunkStore.at(chunkIndex);
			Float h2 = 0;

			// Its _very_ important that we use the _raw weights_ for blending height
			// transitions. If we don't then you will get alterations between different biome
			// heights and huge floating islands due the basisStrength.
			for (auto& biomeWeight : blend.rawWeights) {
				// PERF: Basically every biome height layer is going to want the h0 and
				//       blend info as input, so we pass that in as a param to avoid the
				//       lookup + conversion per biome weight. That does break the
				//       request/generate dependency model though.
				const auto h1 = Engine::withTypeAt<Biomes>(biomeWeight.id, [&]<class Biome>(){
					return generator.get2<typename Biome::Height>(blockCoordX, h0F, blend.info);
				});

				h2 += biomeWeight.weight * h1;
			}

			h2Data[blockRegionIndex] = static_cast<BlockUnit>(std::floor(h2));
		}
	}
}
