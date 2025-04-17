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
#include <Engine/Debug/debug.hpp>
#include <Engine/Debug/PrintScopeIndent.hpp>

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
#include <Game/Terrain/Layer/BiomeBasis.hpp>
#include <Game/Terrain/Layer/BiomeBlock.hpp>
#include <Game/Terrain/Layer/BiomeStructureInfo.hpp>
#include <Game/Terrain/Layer/BiomeStructures.hpp>


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
				Layer::BiomeHeight,
				Layer::BiomeBasis,
				Layer::BiomeBlock,
				Layer::BiomeStructureInfo,
				Layer::BiomeStructures
			>;

			Layers layers;

			// TODO: Add a Pool<T> class for this. Dynamic capacity, dynamic size, but non-destructive on empty/pop.
			size_t currentRequestScope = 0;
			std::vector<Requests<Layers>> requestScopes;

			// TODO: rm
			Layer::BiomeRaw& layerBiomeRaw = std::get<Layer::BiomeRaw>(layers);
			Layer::WorldBaseHeight& layerWorldBaseHeight = std::get<Layer::WorldBaseHeight>(layers);
			Layer::BiomeWeights& layerBiomeWeights = std::get<Layer::BiomeWeights>(layers);
			Layer::BiomeBlended& layerBiomeBlended = std::get<Layer::BiomeBlended>(layers);
			Layer::BiomeHeight& layerBiomeHeight = std::get<Layer::BiomeHeight>(layers);
			Layer::BiomeBasis& layerBiomeBasis = std::get<Layer::BiomeBasis>(layers);
			Layer::BiomeBlock& layerBiomeBlock = std::get<Layer::BiomeBlock>(layers);
			Layer::BiomeStructureInfo& layerBiomeStructureInfo = std::get<Layer::BiomeStructureInfo>(layers);
			Layer::BiomeStructures& layerBiomeStructures = std::get<Layer::BiomeStructures>(layers);

			// TODO: private
			template<class Layer>
			auto& requests() {
				return std::get<Meta::TypeSet::IndexOf<Layers, Layer>::value>(
					requestScopes[currentRequestScope].ranges
				);
			}

			template<class Layer>
			ENGINE_INLINE void request(typename const Layer::Range range) {
				ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- request<{}> range = {}\n", Engine::Debug::ClassName<Layer>(), range);

				requests<Layer>().push_back(range);
				std::get<Layer>(layers).request(range, *this);
			}

			template<class Layer>
			ENGINE_INLINE void requestAwait(typename const Layer::Range range) {
				if (const auto size = requestScopes.size(); currentRequestScope + 1 == size) {
					requestScopes.resize(size + size);
					ENGINE_WARN2("Increasing request depth. Before: {}, After: {}", size, requestScopes.size());
				}

				ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- await<{}> range = {}\n", Engine::Debug::ClassName<Layer>(), range);

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
				ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- generateLayers\n");

				Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
					auto& ranges = requests<Layer>();
					for (const auto& range : ranges) {
						ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- generate<{}> range = {}\n", Engine::Debug::ClassName<Layer>(), range);
						std::get<Layer>(layers).generate(range, *this);
					}
					ranges.clear();
				});
			}

			// TODO: rm - tmep during transition to layers.
			Float rm_getHeight1(const BiomeId id, const BlockUnit blockCoordX, const Float h0, const BiomeRawInfo2& rawInfo, const Float biomeWeight) const;
			Float rm_getBasisStrength(const BiomeId id, const BlockVec blockCoord) const;
			Float rm_getBasis(const BiomeId id, const BlockVec blockCoord) const;
			BlockId rm_getStage(const BiomeId id, const BlockVec blockCoord, const BasisInfo& basisInfo) const;
			void rm_getStructureInfo(const BiomeId id, const ChunkVec chunkCoord, std::back_insert_iterator<std::vector<StructureInfo>> inserter);
			void rm_getStructures(const StructureInfo& info, const RealmId realmId, Terrain& terrain);

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

		public:
			Generator(uint64 seed)
				: layers{
					Layer::WorldBaseHeight{},
					Layer::BiomeRaw{seed},
					Layer::BiomeWeights{},
					Layer::BiomeBlended{},
					Layer::BiomeHeight{},
					Layer::BiomeBasis{},
					Layer::BiomeBlock{},
					Layer::BiomeStructureInfo{},
					Layer::BiomeStructures{},
				} {
				// Arbitrary size, seems like a reasonable default.
				requestScopes.resize(4);
			}

			void generate(Terrain& terrain, const Request& request);

			ENGINE_INLINE auto& getBiomes() noexcept { return biomes; }
			ENGINE_INLINE constexpr static auto getBiomeCount() noexcept { return sizeof...(Biomes); }
			ENGINE_INLINE constexpr auto& getH0Cache() const noexcept { return layerWorldBaseHeight.cache.cache; }

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
			#define TERRAIN_GET_HEIGHT_ARGS \
				const ::Game::BlockUnit blockCoordX, \
				const ::Game::Terrain::Float h0, \
				const ::Game::Terrain::BiomeRawInfo2& rawInfo, \
				const ::Game::Terrain::Float biomeWeight

			#define TERRAIN_GET_BASIS_ARGS \
				const ::Game::BlockVec blockCoord, \
				const ::Game::Terrain::Layer::BiomeHeight& layerBiomeHeight

			#define TERRAIN_GET_LANDMARKS_ARGS \
				const ::Game::ChunkVec& chunkCoord, \
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
				::Game::RealmId realmId, \
				const ::Game::Terrain::StructureInfo& info

			#define TERRAIN_STAGE_ARGS \
				const ::Game::BlockVec blockCoord, \
				const ::Game::Terrain::BasisInfo& basisInfo

			template<StageId CurrentStage, class Biome>
			ENGINE_INLINE BlockId callStage(TERRAIN_STAGE_ARGS);

			template<StageId CurrentStage>
			void generateChunk(Terrain& terrain, Region& region, const RegionIdx regionIdx, const ChunkVec chunkCoord, Chunk& chunk);

	};
}

#include <Game/Terrain/Generator.ipp>
