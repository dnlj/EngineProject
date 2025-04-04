#pragma once

// Game
#include <Game/BlockEntityData.hpp>
#include <Game/BlockMeta.hpp>
#include <Game/MapChunk.hpp> // TODO: Replace/rename/update MapChunk.
#include <Game/Terrain/terrain.hpp>
#include <Game/universal.hpp>

// Engine
#include <Engine/Array.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Math/math.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/StaticVector.hpp>


// TODO: cleanup/remove this file.
#include <Game/Terrain/temp.hpp>

// TODO: fix include dependencies. This uglyness goes away once we remove/split the above classes.
#include <Game/Terrain/Layer/BiomeRaw.hpp>
#include <Game/Terrain/Layer/WorldBaseHeight.hpp>

namespace Game::Terrain {
	// Support for rescaling is needed for preview support. Should not be used for real generation.
	template<class... Biomes>
	class Generator {
		public:
			constexpr static StageId totalStages = std::max({MaxStage<Biomes>::value...});

		private:
			// TODO: One thing to consider is that we loose precision when converting
			//       from BlockCoord to FVec2. Not sure how to solve that other than use
			//       doubles, and that will be slower and still isn't perfect.
			//using FVec2 = glm::vec<2, Float>;

			Layer::BiomeRaw<sizeof...(Biomes)> layerBiomeRaw;
			Layer::WorldBaseHeight layerWorldBaseHeight;

			std::tuple<Biomes...> biomes{};
			static_assert(std::numeric_limits<BiomeId>::max() >= sizeof...(Biomes),
				"More biomes supplied than are currently supported."
			);

			// h0 = broad, world-scale terrain height variations.
			// h1 = biome specific height variations. h1 includes h0 as an input. h1 is
			//      currently only used as part of an intermediate step and not stored
			//      anywhere.
			// h2 = final blended height between all influencing biomes.
			HeightCache h2Cache;

		public:
			Generator(uint64 seed) : layerBiomeRaw{seed} {}

			void generate(Terrain& terrain, const Request& request);

			ENGINE_INLINE auto& getBiomes() noexcept { return biomes; }
			ENGINE_INLINE constexpr static auto getBiomeCount() noexcept { return sizeof...(Biomes); }
			ENGINE_INLINE constexpr auto& getH0Cache() const noexcept { return layerWorldBaseHeight.h0Cache; }

			void setupHeightCaches(const BlockUnit minBlock, const BlockUnit maxBlock);

			/**
			 * Calculate the biome for the given block without any blending/interpolation.
			 */
			[[nodiscard]] BiomeRawInfo calcBiomeRaw(BlockVec blockCoord);

			/**
			 * Get all biome contributions for the given block.
			 */
			[[nodiscard]] BiomeBlend calcBiomeBlend(BlockVec blockCoord, const BlockUnit h0);

			/**
			 * Calculate the final biome for the given block.
			 */
			[[nodiscard]] BiomeBlend calcBiome(BlockVec blockCoord, const BlockUnit h0);

			/**
			 * Calculate the basis terrain info for the given block.
			 * @see BasisInfo
			 */
			[[nodiscard]] BasisInfo calcBasis(const BlockVec blockCoord, const BlockUnit h0);

		private:
			/**
			 * Iterate over each chunk in the request.
			 * The request will be automatically expanded to the correct size for the given stage.
			 */
			template<StageId CurrentStage, class Func>
			ENGINE_INLINE void forEachChunkAtStage(Terrain& terrain, const Request& request, Func&& func);

			// Use Float instead of BlockUnit for height in generator functions since all
			// the generators work in floats and any values should be well within range.

			#define TERRAIN_GET_BASIS_STRENGTH_ARGS \
				const ::Game::BlockVec blockCoord 

			// TODO: Add assert in constructor that checks the Y-independence in constructor.
			// NOTE: Height functions should be Y-independent.
			#define TERRAIN_GET_HEIGHT_ARGS \
				const ::Game::BlockVec blockCoord, \
				const ::Game::Terrain::Float h0, \
				const ::Game::Terrain::BiomeRawInfo& rawInfo, \
				const ::Game::Terrain::Float biomeWeight

			#define TERRAIN_GET_BASIS_ARGS \
				const ::Game::BlockVec blockCoord, \
				const ::Game::Terrain::Float h0, \
				const ::Game::Terrain::Float h2, \
				const ::Game::Terrain::BiomeRawInfo& rawInfo, \
				const ::Game::Terrain::Float biomeWeight

			#define TERRAIN_GET_LANDMARKS_ARGS \
				::Game::Terrain::Terrain& terrain, \
				const ::Game::UniversalRegionCoord& regionCoord, \
				const ::Game::RegionIdx& regionIdx, \
				const ::Game::ChunkVec& chunkCoord, \
				const ::Game::Terrain::Chunk& chunk, \
				/*const ::Game::Terrain::HeightCache& h0Cache, \*/ \
				const ::Game::Terrain::HeightCache& h2Cache, \
				std::back_insert_iterator<std::vector<::Game::Terrain::StructureInfo>> inserter

			// TODO: It might be better to look into some kind of trait/tag based system
			//       for landmark generation instead of having it baked in at the biome
			//       level. That would allow us to say something like:
			//           Boss portals for LavaGuy can spawn anywhere with a temperature > 100deg or with the tag HasLavaGuy.
			//       With genLandmarks that will need to be explicitly included in every
			//       relevant biome. The tag/trait based system would also be useful for generating temporary
			//       entities such as mobs.
			#define TERRAIN_GEN_LANDMARKS_ARGS \
				::Game::Terrain::Terrain& terrain, \
				const ::Game::Terrain::StructureInfo& info

			#define TERRAIN_STAGE_ARGS \
				::Game::Terrain::Terrain& terrain, \
				const ::Game::ChunkVec chunkCoord, \
				const ::Game::BlockVec blockCoord, \
				const ::Game::BlockVec blockIndex, \
				::Game::Terrain::Chunk& chunk, \
				const ::Game::Terrain::BiomeId biomeId, \
				const ::Game::BlockUnit h0, \
				const ::Game::Terrain::BasisInfo& basisInfo

			template<StageId CurrentStage, class Biome>
			ENGINE_INLINE BlockId callStage(TERRAIN_STAGE_ARGS);

			template<StageId CurrentStage>
			void generateChunk(Terrain& terrain, Region& region, const RegionIdx regionIdx, const ChunkVec chunkCoord, Chunk& chunk);

	};
}

#include <Game/Terrain/Generator.ipp>
