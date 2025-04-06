#pragma once

namespace Game::Terrain {
	constexpr inline int64 TestSeed = 1234;

	// TODO: update to use Terrain::Biomes

	template<class...> class Generator;
	using TestGenerator = Generator<
		struct BiomeOne,
		struct BiomeDebugOne,
		struct BiomeDebugTwo,
		struct BiomeDebugThree,
		struct BiomeDebugMountain,
		struct BiomeDebugOcean
	>;
}
