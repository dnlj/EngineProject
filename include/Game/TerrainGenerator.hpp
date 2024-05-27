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
		_last = Small,
		_count,
	};
	ENGINE_BUILD_ALL_OPS(BiomeScale);
	ENGINE_BUILD_DECAY_ENUM(BiomeScale);

	enum class BiomeType : uint8 {
		Default,
		Forest,
		Jungle,
		Taiga,
		Desert,
		Savanna,
		Ocean,
		_count,
	};
	ENGINE_BUILD_DECAY_ENUM(BiomeType);

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

	class BiomeInfo {
		public:
			BiomeScale scale;
			BiomeType type;

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

	//using BiomeStageFunc = void(*)(void* self);
	//
	//template<StageId CurrentStageId, class... Biomes>
	//class BiomeStage {
	//	// If there is a compilation error here your stage<#> overload is likely missing
	//	// `static` or has incorrect arguments.
	//	constexpr static BiomeStageFunc funcs[] = {&Biomes::template stage<CurrentStageId>...};
	//};
	//
	//template<class T, class... Biomes>
	//struct BiomeStages;
	//
	//template<class T, T... Is, class... Biomes>
	//struct BiomeStages<std::integer_sequence<T, Is...>, Biomes...> {
	//	using Tuple = std::tuple<BiomeStage<Is, Biomes...>...>;
	//	//using Tuple = int;
	//};

	template<StageId TotalStages, class... Biomes>
	class Generator {
		private:
			using UInt = uint32;
			using Int = int32;
			using Float = float32;
			using FVec2 = glm::vec<2, Float>;

			// Must be divisible by the previous depth We want to use divisions by three
			// so that each biome can potentially spawn at surface level. If we use two
			// then only the first depth will be at surface level and all others will be
			// above or below it.
			constexpr static BlockUnit biomeScales[] = {
				9000,
				3000,
				1000,
			};

			static_assert(std::size(biomeScales) == +BiomeScale::_count, "Incorrect number of biome scales specified.");
			static_assert(std::ranges::is_sorted(biomeScales, std::greater{}), "Biomes scales should be ordered largest to smallest");

			Engine::Noise::RangePermutation<256> perm;

			// TODO: We need different sample overloads, we want the unit for this to be
			//       BiomeType, but we want to sample with BlockUnit.
			Engine::Noise::RangePermutation<+BiomeType::_count> biomeTypePerm;

		public:
			Generator(uint64 seed)
				: perm{seed}
				, biomeTypePerm{Engine::Noise::lcg(seed)}
			{}
			void generate1(Terrain& terrain, const Request& request);

		private:
			/**
			 * Generate the request for a given stage.
			 * The request will be automatically expanded to the correct size for the given stage.
			 */
			template<StageId CurrentStage>
			void generate(Terrain& terrain, const Request& request);

			template<StageId CurrentStage>
			void generateChunk(Terrain& terrain, const ChunkVec chunkCoord, Chunk& chunk);

			// TODO: this should really be constexpr... this is all static func ptrs
			//using Stages = BiomeStages<std::make_integer_sequence<StageId, TotalStages>, Biomes...>::Tuple stages{};
			std::tuple<Biomes...> biomes{};
			const std::array<void*, sizeof...(Biomes)> biomesErased = { &std::get<Biomes>(biomes)... };

			BiomeInfo calcBiome(const BlockVec blockCoord);
	};
}

#include <Game/TerrainGenerator.ipp>
