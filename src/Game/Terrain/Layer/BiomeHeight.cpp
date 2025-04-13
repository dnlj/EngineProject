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
		ENGINE_LOG2("BiomeHeight::request area=({}, {})", area.min, area.max);
		cache.reset(area.min, area.max);

		generator.requestAwait<WorldBaseHeight>(area);

		{
			auto hMin = generator.get<WorldBaseHeight>(area.min);
			auto hMax = hMin;
			for (auto x = area.min + 1; x < area.max; ++x) {
				const auto h0 = generator.get<WorldBaseHeight>(x);
				hMin = std::min(hMin, h0);
				hMax = std::max(hMax, h0);
			}

			generator.request<BiomeBlended>({
				.min = blockToChunk({area.min, hMin}),
				.max = blockToChunk({area.max, hMax}),
			});
		}

	}

	void BiomeHeight::generate(const Range area, TestGenerator& generator) {
		//
		//
		//
		// TODO: it would be simpler to change the height layers to user chunkX ranges
		//       instead of block. Other systems work in blocks and return ChunkCaches so that
		//       is just a more convinient unit to work with unless we want to do a bunch of conversions.
		//
		//
		//
		for (auto x = area.min + 1; x < area.max; ++x) {
			// TODO: Add static asserts to ensure that the biome height layers _ARE NOT_
			//       cached. Since we don't issue requests cached height layers won't work. They
			//       also don't make sense. The BiomeHeight layer (this layer) is what should be
			//       doing the caching. Other layers should be sampling the BiomeHeight layer, not specific biomes.
			//const auto h0 = generator.get<WorldBaseHeight>(x);
			//const auto blend = generator.get<BiomeBlended>({});

			//
			//
			//
			//
			// TODO: How are we going to avoid over-generation here when blending. We
			//       don't want to generate an entire extra chunk just so we can get a
			//       single block for example. For height specifically its not that bad
			//       since its only the X axis and a few floats don't really cost much.
			//       When we get to basis its going to be bad though. I guess as long as
			//       the basis is cached, but not the biomes. It should be fine. Yeah, if
			//       the biomes aren't cached then there is no over generation.
			//
			//
			//
			//
			//
		}
	}
}
