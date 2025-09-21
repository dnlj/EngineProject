#pragma once

#include <glm/glm.hpp>


#define BIOME_HEIGHT_ARGS \
	const TestGenerator& generator, \
	const ::Game::BlockUnit blockCoordX, \
	const ::Game::Terrain::Float h0, \
	const ::Game::Terrain::RawBiomeInfo& rawInfo

#define BIOME_BASIS_STRENGTH_ARGS \
	const TestGenerator& generator, \
	const ::Game::BlockVec blockCoord

#define BIOME_BASIS_ARGS \
	const TestGenerator& generator, \
	const ::Game::BlockVec blockCoord, \
	const ::Game::BlockUnit h2

#define BIOME_BLOCK_ARGS \
	const TestGenerator& generator, \
	const ::Game::BlockVec blockCoord, \
	const ::Game::Terrain::BasisInfo& basisInfo

#define BIOME_STRUCTURE_INFO_ARGS \
	const TestGenerator& generator, \
	const ::Game::ChunkVec& chunkCoord, \
	std::back_insert_iterator<std::vector<::Game::Terrain::StructureInfo>> inserter

// TODO: It might be better to look into some kind of trait/tag based system
//       for landmark generation instead of having it baked in at the biome
//       level. That would allow us to say something like:
//           Boss portals for LavaGuy can spawn anywhere with a temperature > 100deg or with the tag HasLavaGuy.
//       With genLandmarks that will need to be explicitly included in every
//       relevant biome. The tag/trait based system would also be useful for generating temporary
//       entities such as mobs.
#define BIOME_STRUCTURE_ARGS \
	const TestGenerator& generator, \
	::Game::Terrain::Terrain& terrain, \
	::Game::RealmId realmId, \
	const ::Game::Terrain::StructureInfo& info


namespace Game::Terrain {
	using UInt = uint32;
	using Int = int32;
	using Float = float32;
	using FVec2 = glm::vec<2, Float>;

	using StageId = uint8;
	using BiomeId = uint8;

	ENGINE_INLINE consteval Float operator""_f(const long double v) noexcept { return static_cast<Float>(v); }
	ENGINE_INLINE consteval Float operator""_f(const uint64 v) noexcept { return static_cast<Float>(v); }

	class TestGenerator; // TODO: rm
	using SeqNum = uint64;
}
