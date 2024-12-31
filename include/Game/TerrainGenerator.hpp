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
	using UInt = uint32;
	using Int = int32;
	using Float = float32;

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
	

	constexpr inline int32 biomeBlendDist = 200;
	constexpr inline int32 biomeBlendDist2 = biomeBlendDist / 2;
	static_assert(2 * biomeBlendDist2 == biomeBlendDist);

	class BiomeScaleMeta {
		public:
			BlockUnit size;

			/** Percentage frequency out of [0, 255]. */
			uint8 freq;
	};

	// Each scale must be divisible by the previous depth. We want to use
	// divisions by three so that each biome can potentially spawn at surface
	// level. If we use two then only the first depth will be at surface level and
	// all others will be above or below it. This is due to the division by two
	// for the biomeOffsetY. We need division by two because we want biomes evenly
	// centered on y=0.
	constexpr inline BiomeScaleMeta biomeScales[] = {
		// TODO: Use to be 9000, 3000, 1000. Decreased for easy testing.
		{.size = 3600, .freq = 20},
		{.size = 1200, .freq = 20},
		{.size =  600, .freq = 20},
	};

	static_assert(std::size(biomeScales) == +BiomeScale::_count, "Incorrect number of biome scales specified.");
	static_assert(std::ranges::is_sorted(biomeScales, std::greater{}, &BiomeScaleMeta::size), "Biomes scales should be ordered largest to smallest");
	static_assert(biomeBlendDist <= ((std::end(biomeScales)-1)->size / 2), "Biome blend distances cannot be larger than half the minimum biome size.");

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

	using BiomeId = uint8;

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

	class BiomeRawInfo {
		public:
			BiomeScale scale;
			BiomeId id;
	
			/**
			 * The coordinate in the biome map for this biome.
			 * This is not a block coordinate. This is the coordinate where this biome
			 * would be located in a theoretical grid if all biomes where the same size.
			 */
			BlockVec cell;
	
			/** The remianing block position within the cell */
			BlockVec rem;
	
			// TODO: use a reference, this is currently a pointer to work around the hack
			//       implementation for StaticVector that doesn't correctly use storage +
			//       placement new + destructors. Needed for Generator::calcBiomeBlend
			const BiomeScaleMeta* meta;
	};

	class BiomeWeight {
		public:
			BiomeId id;
			float32 weight;
	};

	using BiomeWeights = Engine::StaticVector<BiomeWeight, 4>;

	void normalizeBiomeWeights(BiomeWeights& weights) {
		const auto total = std::reduce(weights.cbegin(), weights.cend(), 0.0f, [](Float accum, const auto& value){ return accum + value.weight; });
		const auto normF = 1.0f / total;
		for (auto& w : weights) { w.weight *= normF; }
	}

	[[nodiscard]] ENGINE_INLINE BiomeWeight maxBiomeWeight(const BiomeWeights& weights) {
		return *std::ranges::max_element(weights, {}, &BiomeWeight::weight);
	}

	class BasisInfo {
		public:
			BiomeId id;
			float32 basis;
	};

	// TODO: getters with debug bounds checking.
	class Chunk {
		public:
			// TODO: combine id/data arrays? Depends on how muhc data we have once everything is done.
			BlockId data[chunkSize.x][chunkSize.y]{};
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

			BiomeId biomes[biomesPerRegion.x][biomesPerRegion.y]{};

			ENGINE_INLINE constexpr Chunk& chunkAt(RegionIdx regionIdx) noexcept { return chunks[regionIdx.x][regionIdx.y]; }
			ENGINE_INLINE constexpr const Chunk& chunkAt(RegionIdx regionIdx) const noexcept { return chunks[regionIdx.x][regionIdx.y]; }

			ENGINE_INLINE constexpr BiomeId& biomeAt(RegionBiomeIdx regionBiomeIdx) noexcept { return biomes[regionBiomeIdx.x][regionBiomeIdx.y]; }
			ENGINE_INLINE constexpr BiomeId biomeAt(RegionBiomeIdx regionBiomeIdx) const noexcept { return biomes[regionBiomeIdx.x][regionBiomeIdx.y]; }

			/**
			 * Get a unique list of the biome in each corner of this chunk. Used for
			 * during generation. At generation time checking each corner should yield a
			 * unique list of all biomes in this chunk so long as the minimum biome size
			 * is greater than or equal to the chunk size.
			 */
			Engine::StaticVector<BiomeId, 4> getUniqueBiomesApprox(const RegionIdx regionIdx) const noexcept {
				Engine::StaticVector<BiomeId, 4> results;
			
				const auto maybeAdd = [&](BiomeId id) ENGINE_INLINE {
					if (!std::ranges::contains(results, id))  { results.push_back(id); }
				};

				const auto regionBiomeIdx = regionIdxToRegionBiomeIdx(regionIdx);
				constexpr static auto off = biomesPerChunk - 1;
				maybeAdd(biomes[regionBiomeIdx.x][regionBiomeIdx.y]);
				maybeAdd(biomes[regionBiomeIdx.x][regionBiomeIdx.y + off]);
				maybeAdd(biomes[regionBiomeIdx.x + off][regionBiomeIdx.y]);
				maybeAdd(biomes[regionBiomeIdx.x + off][regionBiomeIdx.y + off]);
			
				return results;
			}
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

			// Offset used for sampling biomes so they are roughly centered at ground level.
			//
			// 
			// TODO: vvv update this comment vvv. Ground level will change depending on calc. Do the math. Was +200 instead of +0
			//
			// 
			// Experimentally terrain surface is around 100-300.
			constexpr static BlockUnit biomeOffsetY = biomeScales[0].size / 2 + 0;

			/** Used for sampling the biome frequency. */
			Engine::Noise::RangePermutation<256> biomeFreq;

			// TODO: We need different sample overloads, we want the result unit for this to be
			//       BiomeType, but we want to sample with BlockUnit.
			/** Used for sampling the biome type. */
			Engine::Noise::RangePermutation<256> biomePerm;
			static_assert(sizeof...(Biomes) <= decltype(biomePerm)::size());

			std::tuple<Biomes...> biomes{};
			static_assert(std::numeric_limits<BiomeId>::max() >= sizeof...(Biomes),
				"More biomes supplied than are currently supported."
			);

		public:
			Generator(uint64 seed)
				: biomeFreq{seed}
				, biomePerm{Engine::Noise::lcg(seed)}
			{}

			// TODO: rename
			void generate1(Terrain& terrain, const Request& request);

			ENGINE_INLINE auto& getBiomes() noexcept { return biomes; }

			ENGINE_INLINE constexpr static auto getBiomeCount() noexcept { return sizeof...(Biomes); }
			

			/**
			 * Calculate the biome for the given block without any blending/interpolation.
			 */
			[[nodiscard]] BiomeRawInfo calcBiomeRaw(BlockVec blockCoord);

			/**
			 * Get all biome contributions for the given block.
			 */
			[[nodiscard]] BiomeWeights calcBiomeBlend(BlockVec blockCoord);

			/**
			 * Calcuate the final biome for the given block.
			 */
			[[nodiscard]] BiomeWeights calcBiome(BlockVec blockCoord);

			// TODO: doc
			[[nodiscard]] BasisInfo calcBasis(BlockVec blockCoord);

		private:
			/**
			 * Iterate over each chunk in the request.
			 * The request will be automatically expanded to the correct size for the given stage.
			 */
			template<StageId CurrentStage, class Func>
			ENGINE_INLINE void forEachChunkAtStage(Terrain& terrain, const Request& request, Func&& func);
			
			#define TERRAIN_GET_BASIS_ARGS \
				::Game::BlockVec blockCoord

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
				const ::Game::Terrain::BiomeId biomeId, \
				const ::Game::BlockUnit h0 // The "surface level" offset from zero
			
			template<StageId CurrentStage, class Biome>
			ENGINE_INLINE BlockId callStage(TERRAIN_STAGE_ARGS);

			template<StageId CurrentStage>
			void generateChunk(Terrain& terrain, Region& region, const RegionIdx regionIdx, const ChunkVec chunkCoord, Chunk& chunk);

	};
}

#include <Game/TerrainGenerator.ipp>
