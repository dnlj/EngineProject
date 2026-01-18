#pragma once

// Game
#include <Game/Terrain/Generator.hpp>

// Engine
#include <Engine/tuple.hpp>


namespace Game::Terrain {
	constexpr inline int64 TestSeed = 12345;

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

	// The order biomes are listed is the order they are evaluated, so it is critical that all
	// dependencies for a given layer appear _before_ that layer.
	using Layers = Engine::TupleConcat_t<
		std::tuple<Layer::WorldBaseHeight>,
		std::tuple<Layer::RawBiome>,
		std::tuple<Layer::RawBiomeWeights>,

		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Weight), Biomes>,
		std::tuple<Layer::BlendedBiomeWeights>,

		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Height), Biomes>,
		std::tuple<Layer::BlendedBiomeHeight>,

		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Basis), Biomes>,
		std::tuple<Layer::BlendedBiomeBasis>,

		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Block), Biomes>,
		std::tuple<Layer::BlendedBiomeBlock>,

		//std::tuple<Layer::ChunkBiomeContributions>,		

		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(StructureInfo), Biomes>,
		std::tuple<Layer::BlendedBiomeStructureInfo>,

		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(Structure), Biomes>,
		std::tuple<Layer::BlendedBiomeStructures>
	>;

	using SharedData = Engine::TupleConcat_t<
		Engine::TupleJoinMembersTypesIfExists_t<ENGINE_TRAIT_MEMBER_TYPE_CHECK(SharedData), Biomes>
	>;

	class TestGenerator : public Generator<TestGenerator, Layers, SharedData> {
		using Generator::Generator;
	};
}

#include <Game/Terrain/Layer/BiomeDebug.ipp>
