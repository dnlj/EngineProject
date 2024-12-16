#pragma once

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/StaticVector.hpp>
#include <Engine/Math/math.hpp>

// TODO: rm - once universal coords are moved.
#include <Game/systems/MapSystem.hpp>



// TODO: split out
namespace Game::Terrain {
	using StageId = uint8; // TODO: just stage

	// Should always be ordered largest to smallest
	enum class BiomeScale : uint8 {
		Large,
		Medium,
		Small,
		_count,
		_last = Small,
	};
	ENGINE_BUILD_ALL_OPS(BiomeScale);
	ENGINE_BUILD_DECAY_ENUM(BiomeScale);

	//enum class BiomeType : uint8 {
	//	Default,
	//	Forest,
	//	Jungle,
	//	Taiga,
	//	Desert,
	//	Savanna,
	//	Ocean,
	//	_count,
	//};
	//ENGINE_BUILD_DECAY_ENUM(BiomeType);

	using BiomeId = uint32;

	class StructureInfo {
		public:
			//StructureInfo(BlockVec min, BlockVec max, uint32 id)
			//	: min{min}, max{max}, id{id} {
			//}

			/** Structure min bounds. Inclusive. */
			BlockVec min;

			/** Structure max bounds. */
			BlockVec max;

			/** A id to identify this structure. Defined by each biome. */
			uint32 id;

		//protected:
			bool generate = true;
			RealmId realmId = {};
			BiomeId biomeId = {};
	};

	class BiomeInfo {
		public:
			BiomeScale scale;
			BiomeId id;

			/**
			 * The coordinate in the biome map for this biome.
			 * This is not a block coordinate. This is the coordinate where this biome
			 * would be located in a theoretical grid if all biomes where the same size.
			 */
			BlockVec cell;
	};

	// TODO: getters with debug bounds checking.
	class Chunk {
		public:
			BlockId data[chunkSize.x][chunkSize.y]{};

			//
			//
			//
			//
			// TODO: would using 4x4 make any of the match cheaper? div by two etc.
			//
			//
			//
			//
			/* Grid of biomes in this chunk */
			constexpr static BlockVec chunkBiomesSize = {4, 4};
			BiomeId biomes[chunkBiomesSize.x][chunkBiomesSize.y]{};

			//
			//
			// TODO: Is this still true for 4x4? 
			//
			//
			/**
			 * Get a unique list of the biome in each corner of this chunk.
			 * Used for during generation. At generation time checking each corner should
			 * yield a unique list of all biomes in this chunk.
			 */
			Engine::StaticVector<BiomeId, 4> getUniqueCornerBiomes() const noexcept {
				Engine::StaticVector<BiomeId, 4> results;

				const auto maybeAdd = [&](BiomeId id) {
					if (!std::ranges::contains(results, id)) {
						results.push_back(id);
					}
				};

				constexpr static auto x = chunkBiomesSize.x - 1;
				constexpr static auto y = chunkBiomesSize.y - 1;
				maybeAdd(biomes[0][0]);
				maybeAdd(biomes[0][y]);
				maybeAdd(biomes[x][0]);
				maybeAdd(biomes[x][y]);

				return results;
			}

			BiomeId getBiomeAt(ChunkIdx chunkIndex) const noexcept {
				ENGINE_DEBUG_ASSERT(0 <= chunkIndex.x && chunkIndex.x < chunkSize.x, "Invalid chunk index: ", chunkIndex);
				ENGINE_DEBUG_ASSERT(0 <= chunkIndex.y && chunkIndex.y < chunkSize.y, "Invalid chunk index: ", chunkIndex);
				const auto idx = Engine::Math::divFloor(chunkIndex, chunkBiomesSize).q;
				return biomes[idx.x][idx.y];
			}
	};

	// TODO: getters with debug bounds checking.
	class Region {
		public:
			/** Chunk data for each chunk in the region. */
			Chunk chunks[regionSize.x][regionSize.y]{};

			/** The current stage of each chunk. */
			StageId stages[regionSize.x][regionSize.y]{};

			// TODO: Does this go on the chunk? How do we handle overlaps?
			// TODO: Would it be better to store this on the region and use some kind of
			//       spatial partitioning? BSP/QuadTree/etc.
			//       Don't know if that would help with size/overlap issues. Potentially lots of
			//       structures to check for overlaps, but then again the partitioning would
			//       speed that up quite a bit.
			//StaticVector<Structure, 4> structures;
	};

	class Terrain {
		private:
			Engine::FlatHashMap<UniversalRegionCoord, std::unique_ptr<Region>> regions;

		public:
			Region& getRegion(const UniversalRegionCoord regionCoord) {
				const auto found = regions.find(regionCoord);
				if (found == regions.end()) {
					return *regions.try_emplace(regionCoord, std::make_unique<Region>()).first->second;
				}
				return *found->second;
			}
	};

	class Request {
		public:
			const ChunkVec minChunkCoord;
			const ChunkVec maxChunkCoord;
			const RealmId realmId;

		public:
			// TODO: assert in terrain generator that this won't require generating
			//       chunks, or partial chunks outside of the neighboring regions. That
			//       means that that total number of generation stages + the number of
			//       chunks loaded around a player determine the maximum request
			//       size/limit each other. Although now that we don't generate an entire
			//       region at once it is viable to increase the size of regions if
			//       needed.
			// 
			// TODO: Doc
			// - All chunks are expected to be in the neighborhood of initRegionCoord
			// - 
			//Request(const ChunkVec minChunkCoord, const ChunkVec maxChunkCoord);
	};

	template<class T, StageId Stage>
	concept HasStage = T::template hasStage<Stage>;

	template<class T, StageId Stage = 1>
	struct MaxStage : public std::conditional_t<
		HasStage<T, Stage>,
		MaxStage<T, Stage+1>,
		std::integral_constant<StageId, Stage - 1>
	> {};

	template<class T>
	concept HasLandmarks = requires {
		&T::getLandmarks;
	};

	// Support for rescaling is needed for preview support. Should not be used for real generation.
	template<class... Biomes>
	class Generator {
		public:
			using UInt = uint32;
			using Int = int32;
			using Float = float32;

			constexpr static StageId totalStages = std::max({MaxStage<Biomes>::value...});

		private:
			// TODO: One thing to consider is that we loose precision when converting
			//       from BlockCoord to FVec2. Not sure how to solve that other than use
			//       doubles, and that will be slower and still isn't perfect.
			//using FVec2 = glm::vec<2, Float>;

			class BiomeScaleMeta {
				public:
					BlockUnit scale;

					/** Percentage frequency out of [0, 255]. */
					uint8 freq;
			};

			// Must be divisible by the previous depth. We want to use divisions by three so
			// that each biome can potentially spawn at surface level. If we use two then only
			// the first depth will be at surface level and all others will be above or below
			// it. This is due to the division by two for the biomeOffsetY. We need division
			// by two because we want biomes evenly centered on y=0.
			constexpr static BiomeScaleMeta biomeScales[] = {
				// TODO: Use to be 9000, 3000, 1000. Decreased for easy testing.
				{.scale = 900, .freq = 20},
				{.scale = 300, .freq = 20},
				{.scale = 100, .freq = 20},
			};

			static_assert(std::size(biomeScales) == +BiomeScale::_count, "Incorrect number of biome scales specified.");
			static_assert(std::ranges::is_sorted(biomeScales, std::greater{}, &BiomeScaleMeta::scale), "Biomes scales should be ordered largest to smallest");

			// Offset used for sampling biomes so they are roughly centered at ground level.
			//
			// 
			// TODO: vvv update this comment vvv. Ground level will change depending on calc. Do the math. Was +200 instead of +0
			//
			// 
			// Experimentally terrain surface is around 100-300.
			constexpr static BlockUnit biomeOffsetY = biomeScales[0].scale / 2 + 0;

			/** Used for sampling the biome frequency. */
			Engine::Noise::RangePermutation<256> biomeFreq;

			// TODO: We need different sample overloads, we want the result unit for this to be
			//       BiomeType, but we want to sample with BlockUnit.
			/** Used for sampling the biome type. */
			Engine::Noise::RangePermutation<256> biomePerm;
			static_assert(sizeof...(Biomes) <= decltype(biomePerm)::size());

			std::tuple<Biomes...> biomes{};

		public:
			Generator(uint64 seed)
				: biomeFreq{seed}
				, biomePerm{Engine::Noise::lcg(seed)}
			{}

			void generate1(Terrain& terrain, const Request& request);

			ENGINE_INLINE auto& getBiomes() noexcept { return biomes; }

		private:
			/**
			 * Iterate over each chunk in the request.
			 * The request will be automatically expanded to the correct size for the given stage.
			 */
			template<StageId CurrentStage, class Func>
			ENGINE_INLINE void forEachChunkAtStage(Terrain& terrain, const Request& request, Func&& func);
			
			#define TERRAIN_GET_LANDMARKS_ARGS \
				::Game::Terrain::Terrain& terrain, \
				const ::Game::Terrain::Chunk& chunk, \
				const ::Game::ChunkVec& chunkCoord, \
				std::back_insert_iterator<std::vector<::Game::Terrain::StructureInfo>> inserter

			#define TERRAIN_GEN_LANDMARKS_ARGS \
				::Game::Terrain::Terrain& terrain, \
				const ::Game::Terrain::StructureInfo& info

			#define TERRAIN_STAGE_ARGS \
				::Game::Terrain::Terrain& terrain, \
				const ::Game::ChunkVec chunkCoord, \
				const ::Game::BlockVec blockCoord, \
				const ::Game::BlockVec blockIndex, \
				::Game::Terrain::Chunk& chunk, \
				const ::Game::Terrain::BiomeInfo biomeInfo, \
				const ::Game::BlockUnit h0 // The "surface level" offset from zero
			
			template<StageId CurrentStage, class Biome>
			ENGINE_INLINE BlockId callStage(TERRAIN_STAGE_ARGS);

			template<StageId CurrentStage>
			void generateChunk(Terrain& terrain, const ChunkVec chunkCoord, Chunk& chunk);

			BiomeInfo calcBiome(BlockVec blockCoord);
	};
}

#include <Game/TerrainGenerator.ipp>
