#pragma once

// Engine
#include <Engine/FlatHashMap.hpp>

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

	// TODO: getters with debug bounds checking.
	class Chunk {
		public:
			BlockId data[chunkSize.x][chunkSize.y]{};
	};

	// TODO: getters with debug bounds checking.
	class Region {
		public:
			Chunk chunks[regionSize.x][regionSize.y]{};
			StageId stages[regionSize.x][regionSize.y]{};
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

	using BiomeId = uint32;
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
			 * Generate the request for a given stage.
			 * The request will be automatically expanded to the correct size for the given stage.
			 */
			template<StageId CurrentStage>
			void generate(Terrain& terrain, const Request& request);

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
