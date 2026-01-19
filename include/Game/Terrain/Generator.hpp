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

// STD
#include <atomic>
#include <mutex>
#include <thread>

// TODO: cleanup/remove this file.
#include <Game/Terrain/temp.hpp>

// TODO: fix include dependencies. This uglyness goes away once we remove/split the above classes.
#include <Game/Terrain/Layer/RawBiome.hpp>
#include <Game/Terrain/Layer/RawBiomeWeights.hpp>
#include <Game/Terrain/Layer/WorldBaseHeight.hpp>
#include <Game/Terrain/Layer/BlendedBiomeWeights.hpp>
#include <Game/Terrain/Layer/BlendedBiomeHeight.hpp>
#include <Game/Terrain/Layer/BlendedBiomeBasis.hpp>
#include <Game/Terrain/Layer/BlendedBiomeBlock.hpp>
#include <Game/Terrain/Layer/ChunkBiomeContributions.hpp>
#include <Game/Terrain/Layer/BlendedBiomeStructureInfo.hpp>
#include <Game/Terrain/Layer/BlendedBiomeStructures.hpp>
#include <Game/Terrain/Layer/BiomeOcean.hpp>
#include <Game/Terrain/Layer/BiomeMountain.hpp>

#include <Game/Terrain/Layer/BiomeFoo.hpp>
#include <Game/Terrain/Layer/BiomeDebug.hpp>


namespace Game::Terrain {
	// TODO: Should probably just remove this and add a generic tuple unpack helper.
	template<class LayersSet>
	class RequestsStore;

	template<template<class...> class Set, class... Layers>
	class RequestsStore<Set<Layers...>> {
		public:
			std::tuple<Range<typename Layers::Partition>...> store;
	};

	template<class Self, class Layers, class SharedData>
	class Generator {
		private:
			// nextSeq is update from the main thread and copied to curSeq at generation time.
			std::atomic<SeqNum> nextSeq = 0;

			// Layer data.
			/** All the layers used by this generator. Should be ordered from least dependent to most dependent. */
			Layers layers;
			SharedData sharedData;
			RequestsStore<Layers> requestStore;

			// All threads.
			// NOTE: The coordinator thread and generation threads never have contention since
			//       generateLayers blocks until all generation threads are complete. 
			std::atomic_flag allThreadsShouldExit;
			SeqNum curSeq = 0;
			
			// Coordinator thread.
			std::atomic_flag pending;
			std::thread reqThread;
			std::mutex reqThreadMutex;
			std::condition_variable reqThreadWait;
			uint64 cacheTargetThresholdBytes = 0;
			uint64 cacheMaxThresholdBytes = 0;
			Engine::Clock::Duration cacheTargetTimeout{};
			Engine::FlatHashSet<Layer::BlendedBiomeBlock::Partition> totalBlendedBiomeBlockRequests;
			Engine::FlatHashSet<Layer::BlendedBiomeStructures::Partition> totalBlendedBiomeStructuresRequests;

			// TODO: add a lock-and-swap vector?
			std::vector<Request> genRequestsFront;
			std::vector<Request> genRequestsBack;
			uintz currentLayer = 0;

			// Generation threads.
			std::vector<std::thread> layerGenThreads;
			std::mutex layerGenThreadMutex;
			std::condition_variable layerGenThreadWait;

			// The total remaining generations in progress + in queue.
			std::atomic_int64_t activeLayerGenRemaining = 0;

			// Guarded by the layerGenThreadMutex.
			int64 activeLayerGenNextPartition = -1;
			const void* activeLayerGenPartitions = nullptr;
			using LayerGenerateThreadFuncPtr = void(Generator::*)(decltype(activeLayerGenNextPartition) index);
			LayerGenerateThreadFuncPtr activeLayerGenFunc = nullptr;

			Terrain& terrain;

		public: // TODO: rm/private - Currently public to ease transition to layers architecture in TerrainPreview.
			// TODO: rm - Can't the preview just use `get<Layer>()` ?
			Layer::WorldBaseHeight& layerWorldBaseHeight = std::get<Layer::WorldBaseHeight>(layers); // TODO: Can be removed, currently just for debugging during transition.
			Layer::BlendedBiomeBlock& layerBlendedBiomeBlock = std::get<Layer::BlendedBiomeBlock>(layers);
			Layer::BlendedBiomeStructures& layerBlendedBiomeStructures = std::get<Layer::BlendedBiomeStructures>(layers);

		private:
			Self& self() { return static_cast<Self&>(*this); }
			const Self& self() const { return static_cast<const Self&>(*this); }

			/**
			 * Get the container of all requests for the given layer.
			 */
			template<class Layer>
			auto& requests() { return std::get<Meta::TypeSet::IndexOf<Layers, Layer>::value>(requestStore.store); }

		public:
			Generator(Terrain& terrain, uint64 seed)
				: layers{Engine::makeAll<Layers>(self(), curSeq)}
				, terrain{terrain} {

				const auto& cfg = Engine::getGlobalConfig();
				allocThreads(cfg.cvars.tn_gen_threads);
				setCacheSize(cfg.cvars.tn_gen_cache_target_size, cfg.cvars.tn_gen_cache_max_size);
				setCacheTimeout(cfg.cvars.tn_gen_cache_timeout);
			}

			~Generator() { freeThreads(); }

			void allocThreads(uint64 numGenThreads) {
				freeThreads();

				reqThread = std::thread{&Generator::layerCoordinatorThread, this};

				layerGenThreads.resize(numGenThreads);
				for (auto& thread : layerGenThreads) {
					thread = std::thread{&Generator::layerGenerateThread, this};
				}

				ENGINE_INFO2("Allocated {} threads for terrain generation.", layerGenThreads.size());
			}

			void freeThreads() {
				// Set exit flag and wake all threads.
				allThreadsShouldExit.test_and_set();
				layerGenThreadWait.notify_all();
				reqThreadWait.notify_all();

				// Join all threads.
				for (auto& thread : layerGenThreads) { thread.join(); }
				if (reqThread.joinable()) { reqThread.join(); }

				// Cleanup.
				layerGenThreads.clear();
				allThreadsShouldExit.clear();
			}

			/**
			 * @param targetThreshold The target cache size in MB.
			 * @param maxThreshold The maximum cache size in MB.
			 */
			void setCacheSize(uint64 targetThreshold, uint64 maxThreshold) {
				if (targetThreshold > maxThreshold) {
					ENGINE_WARN2("Terrain generator target threshold must be less than or equal to the max threshold.");
					targetThreshold = maxThreshold;
				}
				
				std::lock_guard lock{reqThreadMutex};
				cacheTargetThresholdBytes = targetThreshold * 1024 * 1024;
				cacheMaxThresholdBytes = maxThreshold * 1024 * 1024;
			}

			void setCacheTimeout(std::chrono::milliseconds timeout) {
				std::lock_guard lock{reqThreadMutex};
				cacheTargetTimeout = timeout;
			}

			[[nodiscard]] ENGINE_INLINE SeqNum getSeq() const noexcept { return curSeq; }

			template<class Layer>
			ENGINE_INLINE_REL void request(typename const Layer::Partition partition) {
				//ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- request<{}> range = {}\n", Engine::Debug::ClassName<Layer>(), range);

				// Ensure correct layer order.
				ENGINE_DEBUG_ASSERT(layerId<Layer>() < currentLayer, "Incorrect layer request order.");

				// We can cut out a lot of duplicate requests by checking the last inserted
				// requests. This tends to come up a lot due to upcasting from block > chunk >
				// region where you have multiple underlying requests that map to the same area.
				auto& reqs = requests<Layer>();
				if (reqs.range.empty() || (reqs.range.back() != partition)) {
					reqs.range.push_back(partition);
				}
			}

			template<class Layer>
			ENGINE_INLINE_REL void request(const Range<typename Layer::Partition>& range) {
				range.forEach([&](const Layer::Partition& partition){
					request<Layer>(partition);
				});
			}

			ENGINE_INLINE void awaitGeneration() {
				//ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- await<{}> range = {}\n", Engine::Debug::ClassName<Layer>(), range);

				{ // Finish requesting lower level layers.
					const auto originalCurrentLayer = currentLayer;

					Engine::forEachReverse(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
						currentLayer = layerId<Layer>();
						if (currentLayer < originalCurrentLayer) {
							processRequests<Layer>();
						}
					});

					currentLayer = originalCurrentLayer;
				}

				// Generate up to the current layer.
				generateLayers();
			}

			template<class Layer>
			void processRequests() {
				auto name = Engine::Debug::ClassName<Layer>(); name;
				if constexpr (requires { &Layer::request; }) {
					const auto& reqs = requests<Layer>();
					if (reqs.range.empty()) { return; }

					//
					//
					//
					// TODO: remove duplicate requests. Use unordered_set?
					//
					//
					//

					std::get<Layer>(layers).request(std::as_const(reqs), self());
				}
			}

			template<class Layer>
			ENGINE_INLINE decltype(auto) get(typename const Layer::Index index) const {
				// TODO: Add dependency checking in debug mode to `get` (and `get2`) calls based on
				//       request scopes to ensure correct dependencies. 
				return std::get<Layer>(layers).get(index);
			}

			// TODO: Is there any value in the normal `get`? I don't think we need Layer::Index anymore.
			template<class Layer, class... Args>
			ENGINE_INLINE decltype(auto) get2(Args&&... args) const {
				return std::get<Layer>(layers).get(self(), std::forward<Args>(args)...);
			}

			void generateLayers() {
				ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- generateLayers\n");

				removeGeneratedRequests();

				Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
					if constexpr (requires { Layer::IsOnDemand; }) { return; }
					if (layerId<Layer>() >= currentLayer) { return; }

					const auto& reqs = requests<Layer>();
					if (reqs.range.empty()) { return; }

					// TODO: Consider using an unordered_set for partitions so we can avoid this
					//       sorting entirely. That would also potentially simplify some of the
					//       flattenRequests by letting use naively insert all partitions and let
					//       the map sort out duplicates. At that point we might be able to just
					//       remove the per-layer flatten all together and add that here instead.
					//
					//       Its unclear if that would actually be better than using a vector though.
					//       Would need more formalized benchmarks/tests. It would also complicate the
					//       thread partition selection and potentially be worse for things such as the
					//       blended layers where sorted partitions are more likely to be spatially
					//       local and in the same biome as well as related cache impacts.
					//
					//       Overall needs a lot of consideration, but potential room for improvement.

					// This ends up just being slower than doing the actual generation (duplicates
					// are caught by the caches. The end result is similar, but just multithreaded.
					// The downside is each layer needs to remember to avoid duplicates, but its 40%
					// faster so that is worth it.
					// 
					//if constexpr (!requires { Layer::IsOnDemand; }) {
					//	std::sort(reqs.partitions.begin(), reqs.partitions.end(), []<class T>(const T& left, const T& right){
					//		if constexpr (std::is_same_v<T, glm::vec<2, int64, glm::packed_highp>>) {
					//			if (left.x < right.x) { return true; }
					//			if (right.x < left.x) { return false; }
					//			if (left.y < right.y) { return true; }
					//			return false;
					//		} else {
					//			return left < right;
					//		}
					//	});
					//
					//	const auto before = reqs.partitions.size();
					//	reqs.partitions.erase(std::unique(reqs.partitions.begin(), reqs.partitions.end()), reqs.partitions.end());
					//	const auto after = reqs.partitions.size();
					//	if (before != after) {
					//		ENGINE_DEBUG2("Before {} After {}", before, after);
					//	}
					//}

					if (!reqs.range.empty()) {
						if constexpr (false /* Single threaded for debugging. */) {
							// NOTE: It's up to each Layer::generate to avoid duplicate data generation. We
							//       can't handle that here because some of the data in the partition could
							//       have been _partially_ generated by a previous request/generation.
							for (const auto& partition : reqs) {
								//ENGINE_DEBUG_PRINT_SCOPE("Generator::Layers", "- generate<{}> partition = {}\n", Engine::Debug::ClassName<Layer>(), partition);
								layer.generate(partition, self());
							}
						} else {
							{
								std::lock_guard lock{layerGenThreadMutex};

								ENGINE_DEBUG_ASSERT(!reqs.range.empty());
								ENGINE_DEBUG_ASSERT(activeLayerGenFunc == nullptr);
								ENGINE_DEBUG_ASSERT(activeLayerGenPartitions == nullptr);
								ENGINE_DEBUG_ASSERT(activeLayerGenRemaining == 0);
								ENGINE_DEBUG_ASSERT(activeLayerGenNextPartition == -1);

								activeLayerGenFunc = &Generator::layerGenerateLayer<Layer>;
								activeLayerGenPartitions = &reqs;
								activeLayerGenRemaining = std::ssize(reqs.range);
								activeLayerGenNextPartition = activeLayerGenRemaining - 1;
								layerGenThreadWait.notify_all();
							}

							while (true) {
								const auto rem = activeLayerGenRemaining.load();
								if (rem == 0) { break; }
								ENGINE_DEBUG_ASSERT(rem > 0);
								activeLayerGenRemaining.wait(rem);
							}

							activeLayerGenFunc = nullptr;
							activeLayerGenPartitions = nullptr;
						}
					}
				});

				clearRequests();
			}

			template<class Data>
			auto const& shared() const noexcept {
				return std::get<Data>(sharedData);
			}

		public:
			/**
			 * Queue a request for generation.
			 * The request queue does not begin processing new requests until submit() is called.
			 */
			void generate(const Request& request);

			/**
			 * Begin processing the current request queue.
			 * 
			 * @param seq The sequence number to use for this batch of requests. The sequence
			 *            numbers are used for cache cleanup purposes.
			 */
			void submit(Engine::Clock::TimePoint time) {
				nextSeq = time.time_since_epoch().count();
				reqThreadWait.notify_one();
			}
			
			/**
			 * Is the Generator actively processing requests (or has any pending).
			 * Should rarely be needed. Only used for things that need direct access the
			 * generator directly such as the TerrainPreview.
			 */
			bool isPending() const { return pending.test(); }

		private:
			template<class Layer>
			consteval static uintz layerId() noexcept { return Meta::TypeSet::IndexOf<Layers, Layer>::value; }
			consteval static uintz layerIdEnd() noexcept { return std::tuple_size_v<Layers>; }

			/**
			 * Clears any active generated requests.
			 * Run exclusively from generateLayers on the coordinator thread.
			 * @see generateLayers
			 */
			void clearRequests() {
				// TODO: would like a better, non layer-specific way to handle this if possible.
				//       Maybe a layer flag that can say if we should capture all requests?
				//       IsTopLevelLayer? or similar.

				if (layerId<Layer::BlendedBiomeBlock>() >= currentLayer) {
					auto& range = requests<Layer::BlendedBiomeBlock>().range;
					totalBlendedBiomeBlockRequests.insert(range.begin(), range.end());
				}

				if (layerId<Layer::BlendedBiomeStructures>() >= currentLayer) {
					auto& range = requests<Layer::BlendedBiomeStructures>().range;
					totalBlendedBiomeStructuresRequests.insert(range.begin(), range.end());
				}

				Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
					if (layerId<Layer>() >= currentLayer) { return; }
					requests<Layer>().range.clear();
				});
			}
			
			/**
			 * Removes any already generated requests.
			 * Run exclusively from generateLayers on the coordinator thread.
			 * @see generateLayers
			 */
			void removeGeneratedRequests() {
				//Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
				//	if constexpr (requires { Layer::IsOnDemand; }) { return; }
				//	auto& reqs = requests<Layer>();
				//	std::sort(reqs.begin(), reqs.end());
				//	reqs.erase(std::unique(reqs.begin(), reqs.end()), reqs.end());
				//});

				//
				//
				// TODO: This actually can't work unless we add extra logic... It can cause us to
				//       miss areas when copying to terrain data. So we need a way to avoid that.
				//       Maybe just copy to a diff structure?
				//
				//
				
				//Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
				//	if constexpr (requires { Layer::IsOnDemand; }) { return; }
				//
				//	auto& reqs = requests<Layer>();
				//	if (reqs.empty()) { return; }
				//
				//	//ENGINE_DEBUG_ONLY(const auto _debugBefore = reqs.size());
				//
				//	// NOTE: removeGenerated does _not_ remove the need for `cache.populated(...)`
				//	//       since the partition unit is not necessarily the same as the cache unit.
				//	layer.removeGenerated(reqs);
				//
				//	//ENGINE_DEBUG_ONLY(const auto _debugAfter = reqs.size());
				//
				//	// Very helpful for debugging and optimizing requests/partitions.
				//	//ENGINE_DEBUG2("Reqs {} | Ranges: {} | before: {} after: {} total: {}",
				//	//	Engine::Debug::ClassName<Layer>(),
				//	//	//reqs.ranges,
				//	//	reqs.ranges.size(),
				//	//	_debugBefore,
				//	//	_debugAfter,
				//	//	_debugBefore - _debugAfter
				//	//);
				//});
			}

			/**
			 * Clear the layer caches if needed based on cache target and max thresholds.
			 * Run exclusively from the coordinator thread.
			 * @see setCacheSize
			 */
			void cleanCaches();

			void layerCoordinatorThread();

			void layerGenerateThread() {
				while (true) {
					std::unique_lock lock{layerGenThreadMutex};

					// TODO: Consider adding our own jthread/stop_source/wait(stop_token)
					//       equivalent for nice stop handling.
					while ((activeLayerGenNextPartition < 0) && !allThreadsShouldExit.test()) {
						layerGenThreadWait.wait(lock);
					}

					// Exit thread.
					if (allThreadsShouldExit.test()) { return; }

					// Get the index for the next partition to process.
					const auto index = activeLayerGenNextPartition;
					--activeLayerGenNextPartition;

					// Now that we have the next partition, free the lock so other threads can
					// process other partitions. We don't want to keep it locked while
					// generating.
					lock.unlock();

					// Generate the layer data.
					ENGINE_DEBUG_ASSERT(activeLayerGenFunc != nullptr, "Attempting to generate layers with no active requests.");
					std::invoke(activeLayerGenFunc, this, index);

					// Let everyone know we are done. Don't notify unless we are the last partition.
					static_assert(decltype(activeLayerGenRemaining)::is_always_lock_free);

					if (activeLayerGenRemaining.fetch_sub(1) == 1) {
						activeLayerGenRemaining.notify_one();
					}
				}
			}

			template<class Layer>
			void layerGenerateLayer(int64 index) {
				// Get the next partition.
				const auto* partitions = reinterpret_cast<const std::vector<typename Layer::Partition>*>(activeLayerGenPartitions);
				ENGINE_DEBUG_ASSERT(partitions != nullptr);
				ENGINE_DEBUG_ASSERT(!partitions->empty());

				// Generate the layer partition data.
				std::get<Layer>(layers).generate((*partitions)[index], self());
			}

			/**
			 * Process requests and generate the layers.
			 * Run exclusively from the coordinator thread.
			 */
			void processGenRequests();
	};
}

#include <Game/Terrain/Generator.ipp>
