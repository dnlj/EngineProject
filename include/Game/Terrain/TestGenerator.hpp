#include <Game/Terrain/Generator.hpp>
#include <Game/Terrain/biomes/BiomeOne.hpp>
#include <Game/Terrain/biomes/BiomeDebug.hpp>


namespace Game::Terrain {
	constexpr inline int64 TestSeed = 1234;
	using TestGenerator = Generator<
		BiomeOne,
		BiomeDebugOne,
		BiomeDebugTwo,
		BiomeDebugThree,
		BiomeDebugMountain,
		BiomeDebugOcean
	>;
}
