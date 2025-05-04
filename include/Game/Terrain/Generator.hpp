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

#include <Game/Terrain/Layer/BiomeFoo.hpp>
#include <Game/Terrain/Layer/BiomeDebug.hpp>


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
	// TODO: Can we reframe the generator as a top level layer? I don't think so, not
	//       quite. How would we do things like apply structures over block data?
	//
	//
	//
	// Support for rescaling is needed for preview support. Should not be used for real generation.
	template<class... Biomes>
	class Generator {
		private:
			// TODO: One thing to consider is that we loose precision when converting
			//       from BlockCoord to FVec2. Not sure how to solve that other than use
			//       doubles, and that will be slower and still isn't perfect.
			//using FVec2 = glm::vec<2, Float>;

		public: // TODO: rm/private - Currently public to ease transition to layers architecture in TerrainPreview.
			//
			//
			//
			//
			// TODO: static assert layer dependency order
			//
			//
			//
			//

			// TODO: auto unpack biomes into `Layers` tuple
			// TODO: rename
			// TODO: For the time being these need to match the exact order as in TestGenerator until layer transition is complete.
			using Biomes2 = std::tuple<
				Layer::BiomeFoo,
				Layer::BiomeDebugOne,
				Layer::BiomeDebugTwo,
				Layer::BiomeDebugThree,
				Layer::BiomeDebugMountain,
				Layer::BiomeDebugOcean
			>;

			//
			//
			//
			//
			// TODO: rename the biome accumulation layers to something other than BiomeX to avoid conflict with actual biomes.
			//
			//
			//
			using Layers = std::tuple<
				Layer::WorldBaseHeight,

				// TODO: how to structure and init biome layeres.
				Layer::BiomeFoo::Height,
				Layer::BiomeDebugOne::Height,
				Layer::BiomeDebugTwo::Height,
				Layer::BiomeDebugThree::Height,
				Layer::BiomeDebugMountain::Height,
				Layer::BiomeDebugOcean::Height,

				Layer::BiomeFoo::BasisStrength,
				Layer::BiomeDebugOne::BasisStrength,
				Layer::BiomeDebugTwo::BasisStrength,
				Layer::BiomeDebugThree::BasisStrength,
				Layer::BiomeDebugMountain::BasisStrength,
				Layer::BiomeDebugOcean::BasisStrength,

				Layer::BiomeFoo::Basis,
				Layer::BiomeDebugOne::Basis,
				Layer::BiomeDebugTwo::Basis,
				Layer::BiomeDebugThree::Basis,
				Layer::BiomeDebugMountain::Basis,
				Layer::BiomeDebugOcean::Basis,

				Layer::BiomeFoo::Block,
				Layer::BiomeDebugOne::Block,
				Layer::BiomeDebugTwo::Block,
				Layer::BiomeDebugThree::Block,
				Layer::BiomeDebugMountain::Block,
				Layer::BiomeDebugOcean::Block,

				Layer::BiomeFoo::StructureInfo,

				Layer::BiomeFoo::Structure,

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
				ENGINE_DEBUG_ASSERT(!range.empty(), "Attempting to request empty layer range.");
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

			//
			//
			//
			//
			//
			// TODO: Is there any value in the normal `get`? I don't think we need Layer::Index anymore.
			//
			//
			//
			//
			//
			//
			template<class Layer, class... Args>
			ENGINE_INLINE decltype(auto) get2(Args&&... args) const {
				return std::get<Layer>(layers).get(std::forward<Args>(args)...);
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
					
					Layer::BiomeFoo::Height{},
					Layer::BiomeDebugOne::Height{},
					Layer::BiomeDebugTwo::Height{},
					Layer::BiomeDebugThree::Height{},
					Layer::BiomeDebugMountain::Height{},
					Layer::BiomeDebugOcean::Height{},

					Layer::BiomeFoo::BasisStrength{},
					Layer::BiomeDebugOne::BasisStrength{},
					Layer::BiomeDebugTwo::BasisStrength{},
					Layer::BiomeDebugThree::BasisStrength{},
					Layer::BiomeDebugMountain::BasisStrength{},
					Layer::BiomeDebugOcean::BasisStrength{},

					Layer::BiomeFoo::Basis{},
					Layer::BiomeDebugOne::Basis{},
					Layer::BiomeDebugTwo::Basis{},
					Layer::BiomeDebugThree::Basis{},
					Layer::BiomeDebugMountain::Basis{},
					Layer::BiomeDebugOcean::Basis{},

					Layer::BiomeFoo::Block{},
					Layer::BiomeDebugOne::Block{},
					Layer::BiomeDebugTwo::Block{},
					Layer::BiomeDebugThree::Block{},
					Layer::BiomeDebugMountain::Block{},
					Layer::BiomeDebugOcean::Block{},

					Layer::BiomeFoo::StructureInfo{},

					Layer::BiomeFoo::Structure{},

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

	};
}

#include <Game/Terrain/Generator.ipp>
