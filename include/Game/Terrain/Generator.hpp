#pragma once

// TODO: cleanup includes

// Game
#include <Game/BlockEntityData.hpp>
#include <Game/BlockMeta.hpp>
#include <Game/MapChunk.hpp> // TODO: Replace/rename/update MapChunk.
#include <Game/Terrain/Request.hpp>
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
#include <Game/Terrain/Layer/BiomeOcean.hpp>
#include <Game/Terrain/Layer/BiomeMountain.hpp>

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

	template<class Self, class Layers, class SharedData>
	class Generator {
		private:
			// TODO: One thing to consider is that we loose precision when converting
			//       from BlockCoord to FVec2. Not sure how to solve that other than use
			//       doubles, and that will be slower and still isn't perfect.
			//using FVec2 = glm::vec<2, Float>;
			
			Self& self() { return static_cast<Self&>(*this); }
			const Self& self() const { return static_cast<const Self&>(*this); }

		public: // TODO: rm/private - Currently public to ease transition to layers architecture in TerrainPreview.
			
			Layers layers{};
			SharedData sharedData{};

			// TODO: Add a Pool<T> class for this. Dynamic capacity, dynamic size, but non-destructive on empty/pop.
			size_t currentRequestScope = 0;
			std::vector<Requests<Layers>> requestScopes;

			// TODO: rm - Can't the preview just use `get<Layer>()` ?
			Layer::BiomeRaw& layerBiomeRaw = std::get<Layer::BiomeRaw>(layers);
			Layer::WorldBaseHeight& layerWorldBaseHeight = std::get<Layer::WorldBaseHeight>(layers);
			Layer::BiomeHeight& layerBiomeHeight = std::get<Layer::BiomeHeight>(layers);
			Layer::BiomeBlock& layerBiomeBlock = std::get<Layer::BiomeBlock>(layers);
			Layer::BiomeStructureInfo& layerBiomeStructureInfo = std::get<Layer::BiomeStructureInfo>(layers);
			Layer::BiomeStructures& layerBiomeStructures = std::get<Layer::BiomeStructures>(layers);

		private:
			template<class Layer>
			auto& requests() {
				return std::get<Meta::TypeSet::IndexOf<Layers, Layer>::value>(
					requestScopes[currentRequestScope].ranges
				);
			}

		public:
			template<class Layer>
			ENGINE_INLINE void request(typename const Layer::Range range) {
				ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- request<{}> range = {}\n", Engine::Debug::ClassName<Layer>(), range);
				ENGINE_DEBUG_ASSERT(!range.empty(), "Attempting to request empty layer range.");
				requests<Layer>().push_back(range);
				std::get<Layer>(layers).request(range, self());
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

			// TODO: Is there any value in the normal `get`? I don't think we need Layer::Index anymore.
			template<class Layer, class... Args>
			ENGINE_INLINE decltype(auto) get2(Args&&... args) const {
				return std::get<Layer>(layers).get(self(), std::forward<Args>(args)...);
			}

			void generateLayers() {
				// TODO: optimize/combine/remove overlapping and redundant requests.
				ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- generateLayers\n");

				Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
					auto& ranges = requests<Layer>();
					for (const auto& range : ranges) {
						ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- generate<{}> range = {}\n", Engine::Debug::ClassName<Layer>(), range);
						std::get<Layer>(layers).generate(range, self());
					}
					ranges.clear();
				});
			}

			template<class Data>
			auto const& shared() const noexcept {
				return std::get<Data>(sharedData);
			}

		public:
			Generator(uint64 seed) {
				// Arbitrary size, seems like a reasonable default.
				requestScopes.resize(4);
			}

			void generate(Terrain& terrain, const Request& request);

			ENGINE_INLINE constexpr auto& getH0Cache() const noexcept { return layerWorldBaseHeight.cache; }

	};
}

#include <Game/Terrain/Generator.ipp>
