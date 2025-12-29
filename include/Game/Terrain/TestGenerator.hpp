#pragma once

// Game
#include <Game/Terrain/Generator.hpp>

// Engine
#include <Engine/tuple.hpp>


namespace Game::Terrain {
	constexpr inline int64 TestSeed = 1234;

	// TODO: Note somewhere that biomes don't need generation/partition since they are on
	//       demand. The caching is done at the BlendedBiomeBlock level. May want to consider a
	//       different structure for biomes since half of the Layer functionality isn't
	//       used?
	using Biomes = std::tuple<
		Layer::BiomeFoo,
		Layer::BiomeDebugOne,
		Layer::BiomeDebugTwo,
		Layer::BiomeDebugThree,
		Layer::BiomeDebugMountain,
		Layer::BiomeOcean
	>;
	constexpr inline auto biomeCount = std::tuple_size_v<Biomes>;
	
	using Layers = Engine::TupleConcat_t<
		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Height), Biomes>,
		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Weight), Biomes>,
		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Basis), Biomes>,
		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Block), Biomes>,
		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(StructureInfo), Biomes>,
		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Structure), Biomes>,

		// TODO: Really these should be interleaved with the biome layers, but atm the
		//       biome layers are immediate (no caching) so it doesn't matter.
		std::tuple<
			Layer::WorldBaseHeight,
			Layer::RawBiome,
			Layer::RawBiomeWeights,
			Layer::BlendedBiomeWeights,
			Layer::BlendedBiomeHeight,
			Layer::BlendedBiomeBasis,
			Layer::BlendedBiomeBlock,
			Layer::BlendedBiomeStructureInfo,
			Layer::BlendedBiomeStructures
		>
	>;

	using SharedData = Engine::TupleConcat_t<
		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(SharedData), Biomes>
	>;

	class TestGenerator : public Generator<TestGenerator, Layers, SharedData> {
		using Generator::Generator;
	};
}

#include <Game/Terrain/Layer/BiomeDebug.ipp>
