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
	template<class Layer>
	class RequestSet {
		public:
			std::vector<typename Layer::Range> ranges;
			std::vector<typename Layer::Partition> partitions;

			ENGINE_INLINE void clear() {
				ranges.clear();
				partitions.clear();
			}

			ENGINE_INLINE bool empty() const {
				return ranges.empty();
			}
	};

	// TODO: Should probably just remove this and add a generic tuple unpack helper.
	template<class LayersSet>
	class Requests;

	template<template<class...> class Set, class... Layers>
	class Requests<Set<Layers...>> {
		public:
			std::tuple<RequestSet<Layers>...> store;
	};

	template<class Self, class Layers, class SharedData>
	class Generator {
		private:
			// TODO: One thing to consider is that we loose precision when converting
			//       from BlockCoord to FVec2. Not sure how to solve that other than use
			//       doubles, and that will be slower and still isn't perfect.
			//using FVec2 = glm::vec<2, Float>;

			Layers layers{};
			SharedData sharedData{};

			// TODO: Add a Pool<T> class for this. Dynamic capacity, dynamic size, but non-destructive on empty/pop.
			size_t currentRequestScope = 0;
			std::vector<Requests<Layers>> requestScopes;

		public: // TODO: rm/private - Currently public to ease transition to layers architecture in TerrainPreview.
			// TODO: rm - Can't the preview just use `get<Layer>()` ?
			Layer::WorldBaseHeight& layerWorldBaseHeight = std::get<Layer::WorldBaseHeight>(layers); // TODO: Can be removed, currently just for debugging during transition.
			Layer::BiomeBlock& layerBiomeBlock = std::get<Layer::BiomeBlock>(layers);
			Layer::BiomeStructures& layerBiomeStructures = std::get<Layer::BiomeStructures>(layers);

		private:
			Self& self() { return static_cast<Self&>(*this); }
			const Self& self() const { return static_cast<const Self&>(*this); }

			/**
			 * Get the container of all requests for all layers in the current request scope.
			 */
			template<class Layer>
			auto& requests() {
				return std::get<Meta::TypeSet::IndexOf<Layers, Layer>::value>(
					requestScopes[currentRequestScope].store
				);
			}

		public:
			template<class Layer>
			ENGINE_INLINE void request(typename const Layer::Range range) {
				ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- request<{}> range = {}\n", Engine::Debug::ClassName<Layer>(), range);
				ENGINE_DEBUG_ASSERT(!range.empty(), "Attempting to request empty layer range.");
				requests<Layer>().ranges.push_back(range);
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
			
			template<class Layer>
			ENGINE_INLINE decltype(auto) get3(const auto& index) const {
				return std::get<Layer>(layers).get(index);
			}

			void generateLayers() {
				ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- generateLayers\n");

				Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
					auto& reqs = requests<Layer>();
					if (reqs.empty()) { return; }

					ENGINE_DEBUG_ASSERT(reqs.partitions.empty(), "Unexpected partitions already populated.");
					layer.partition(reqs.ranges, reqs.partitions);

					// NOTE: It's up to each layer.generate to avoid duplicate data
					//       generation. We can't handle that here because some of the
					//       data in the partion could have been _partially_ generated by
					//       a previous request/generation.
					for (const auto& partition : reqs.partitions) {
						ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- generate<{}> partition = {}\n", Engine::Debug::ClassName<Layer>(), partition);
						layer.generate(partition, self());
					}

					reqs.clear();
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

	};
}

#include <Game/Terrain/Generator.ipp>
