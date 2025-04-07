// TODO: need biome blend first
/*
// Game
#include <Game/Terrain/Layer/BiomeHeight.hpp>

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
		ENGINE_LOG2("BiomeHeight::request area=({}, {})", area.xMin, area.xMax);
		cache.reset(area.xMin, area.xMax);

		// TODO: h0 request
	}

	void BiomeHeight::generate(const Range area, TestGenerator& generator) {
	}
}
*/
