// Game
#include <Game/Terrain/Layer/BiomeHeight.hpp>
#include <Game/Terrain/Layer/BiomeBlended.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>
#include <Game/Terrain/biomes/all.hpp>

namespace Game::Terrain::Layer {
	void BiomeHeight::request(const Range area, TestGenerator& generator) {
		// TODO: Reset isn't quite right here. It could/will be possible to have
		//       multiple requests "active" at once/threaded. This is fine
		//       currently because we happen to only have one request at
		//       once/single thread. We will need to revisit this once we get to
		//       multithreading and request optimization.
		cache.reserve(area);

		generator.requestAwait<WorldBaseHeight>(area);

		{
			const auto blockMinX = area.min * blocksPerChunk;
			const auto blockMaxX = area.max * blocksPerChunk;
			auto hMin = generator.get<WorldBaseHeight>(blockMinX);
			auto hMax = hMin;
			for (auto x = blockMinX + 1; x < blockMaxX; ++x) {
				const auto h0 = generator.get<WorldBaseHeight>(x);
				hMin = std::min(hMin, h0);
				hMax = std::max(hMax, h0);
			}

			generator.request<BiomeBlended>({
				.min = blockToChunk({blockMinX, hMin}),
				.max = blockToChunk({blockMaxX, hMax}) + ChunkVec{0, 1}, // Add one to get an _exlusive_ bound instead of inclusive.
			});

			// TODO: We should probably be doing this with correct `request` calls to the blended biomes.
			// 
			// Note that we don't send requests to the biome in the blend. In theory we
			// should be doing a requestAwait for BiomeBlended and then also sending
			// requests to each biome in the resulting blend. Currently this is "okay" to
			// skip because all biome layers are expected to resolve in `get` anyways.
			// Although this does bypass the request/generate dependency model.
		}

	}

	void BiomeHeight::generate(const Range area, TestGenerator& generator) {
		// TODO: Add static asserts to ensure that the biome height layers _ARE NOT_
		//       cached. Since we don't issue requests cached height layers won't work. They
		//       also don't make sense. The BiomeHeight layer (this layer) is what should be
		//       doing the caching. Other layers should be sampling the BiomeHeight layer, not specific biomes.

		auto cur = cache.walk(area);
		while (cur) {
			const auto blockCoordX = cur.getBlockCoord();
			const auto h0 = generator.get<WorldBaseHeight>(blockCoordX);
			const auto h0F = static_cast<Float>(h0);
			const auto chunkCoord = blockToChunk({blockCoordX, h0});
			const auto& chunkStore = generator.get<BiomeBlended>(chunkCoord);
			const auto chunkIndexY = blockToChunkIndex({0, h0}, chunkCoord).y;
			const auto& blend = chunkStore.at({cur.getChunkIndex(), chunkIndexY});
			Float h2 = 0;

			// Its _very_ important that we use the _raw weights_ for blending height
			// transitions. If we don't then you will get alterations between different biome
			// heights and huge floating islands due the basisStrength.
			for (auto& biomeWeight : blend.rawWeights) {
				// PERF: Basically every biome height layer is going to want the h0 and
				//       blend info as input, so we pass that in as a param to avoid the
				//       lookup + conversion per biome weight. That does break the
				//       request/generate dependency model though.
				const auto h1 = Engine::withTypeAt<TestGenerator::Biomes2>(biomeWeight.id, [&]<class Biome>(){
					return generator.get2<typename Biome::Height>(blockCoordX, h0F, blend.info);
				});

				h2 += biomeWeight.weight * h1;
			}

			cache.at(blockCoordX) = static_cast<BlockUnit>(std::floor(h2));
			++cur;
		}
	}
}
