#pragma once

// TODO: cleanup includes

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
#include <Engine/tuple.hpp>

// Meta
#include <Meta/TypeSet/IndexOf.hpp>

// TODO: cleanup/remove this file.
#include <Game/Terrain/temp.hpp>

// TODO: fix include dependencies. This uglyness goes away once we remove/split the above classes.
#include <Game/Terrain/Layer/BiomeRaw.hpp>
#include <Game/Terrain/Layer/BiomeWeights.hpp>
#include <Game/Terrain/Layer/WorldBaseHeight.hpp>
#include <Game/Terrain/Layer/BiomeBlended.hpp>
#include <Game/Terrain/Layer/BiomeHeight.hpp>


namespace Game::Terrain {
	

	template<class LayersSet>
	class Requests;

	template<template<class...> class Set, class... Layers>
	class Requests<Set<Layers...>> {
		public:
			std::tuple<std::vector<typename Layers::Range>...> ranges;
	};

	//
	//
	//
	// TODO: Can we reframe the generator as a top level layer? I think so, or at least have one as a member.
	//
	//
	//
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

		public: // TODO: rm/private - Currently public to ease transition to layers architecture in TerrainPreview.
			// TODO: static assert layer dependency order
			using Layers = std::tuple<
				Layer::WorldBaseHeight,
				Layer::BiomeRaw,
				Layer::BiomeWeights,
				Layer::BiomeBlended,
				Layer::BiomeHeight
			>;

			Layers layers;
			//Requests<Layers> requests; // TODO: flatten to just a type helper if no functions are needed.


			// TODO: Add a Pool<T> class for this. Dynamic capacity, dynamic size, but non-destructive on empty/pop.
			size_t currentRequestScope = 0;
			std::vector<Requests<Layers>> requestScopes;

			// TODO: rm
			Layer::BiomeRaw& layerBiomeRaw = std::get<Layer::BiomeRaw>(layers);
			Layer::WorldBaseHeight& layerWorldBaseHeight = std::get<Layer::WorldBaseHeight>(layers);
			Layer::BiomeWeights& layerBiomeWeights = std::get<Layer::BiomeWeights>(layers);
			Layer::BiomeBlended& layerBiomeBlended = std::get<Layer::BiomeBlended>(layers);

			// TODO: private
			template<class Layer>
			auto& requests() {
				return std::get<Meta::TypeSet::IndexOf<Layers, Layer>::value>(
					requestScopes[currentRequestScope].ranges
				);
			}

			template<class Layer>
			ENGINE_INLINE void request(typename const Layer::Range range) {
				requests<Layer>().push_back(range);
				std::get<Layer>(layers).request(range, *this);
			}

			template<class Layer>
			ENGINE_INLINE void requestAwait(typename const Layer::Range range) {
				if (const auto size = requestScopes.size(); currentRequestScope + 1 == size) {
					requestScopes.resize(size + size);
					ENGINE_WARN2("Increasing request depth. Before: {}, After: {}", size, requestScopes.size());
				}

				++currentRequestScope;
				request<Layer>(range);
				generateLayers();
				requests<Layer>().clear();
				--currentRequestScope;
			}

			template<class Layer>
			ENGINE_INLINE decltype(auto) get(typename const Layer::Index index) const {
				return std::get<Layer>(layers).get(index);
			}

			void generateLayers() {
				// TODO: optimize/combine/remove overlapping and redundant requests.
				Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
					auto& ranges = requests<Layer>();
					for (const auto& range : ranges) {
						std::get<Layer>(layers).generate(range, *this);
					}
					ranges.clear();
				});
			}

			// TODO: rm - tmep during transition to layers.
			Float rm_getBasisStrength(BiomeId id, BlockVec blockCoord) const;

		private:
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
			Generator(uint64 seed)
				: layers{
					Layer::WorldBaseHeight{},
					Layer::BiomeRaw{seed},
					Layer::BiomeWeights{},
					Layer::BiomeBlended{},
					Layer::BiomeHeight{},
				} {
				// Arbitrary size, seems like a reasonable default.
				requestScopes.resize(4);
			}

			void generate(Terrain& terrain, const Request& request);

			ENGINE_INLINE auto& getBiomes() noexcept { return biomes; }
			ENGINE_INLINE constexpr static auto getBiomeCount() noexcept { return sizeof...(Biomes); }
			ENGINE_INLINE constexpr auto& getH0Cache() const noexcept { return layerWorldBaseHeight.h0Cache; }

			void setupHeightCaches(const BlockUnit minBlock, const BlockUnit maxBlock);

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
				const ::Game::Terrain::BiomeRawInfo2& rawInfo, \
				const ::Game::Terrain::Float biomeWeight

			#define TERRAIN_GET_BASIS_ARGS \
				const ::Game::BlockVec blockCoord, \
				const ::Game::Terrain::Float h0, \
				const ::Game::Terrain::Float h2, \
				const ::Game::Terrain::BiomeRawInfo2& rawInfo, \
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
