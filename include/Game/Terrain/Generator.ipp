#pragma once

#include <Game/Terrain/Generator.hpp>


namespace Game::Terrain {
	template<class Self, class Layers, class SharedData>
	void Generator<Self, Layers, SharedData>::generate(const Request& request) {
		// TODO: Move/redocument things in terms of layers once transition is done.
		// - Generate stages.
		//   - Stage 1, Stage 2, ..., Stage N.
		// - Generate candidate features for all biomes in the request.
		// - Cull candidate features based on point (or AABB?) and realized biome.
		// - Generate non-culled features based on resolved biome.
		// - Generate decorations.
		//   - Basically same as stages.
		//   - Provides a place to smooth/integrate features and terrain.
		//   - Moss, grass tufts, cobwebs, chests/loot, etc.
		//   - Do these things really need extra passes? Could this be done during the initial stages and feature generation?

		// TODO: We should have a third queue on the main thread so that we don't need to
		//       lock here? Then we could have a dedicate swap function once all requests
		//       are done + cv. Its not quite that simple because we don't know that the
		//       back queue has finished processing at the time we swap. So we would need
		//       to instead append to each list. Unclear if that would be better.
		
		{
			std::lock_guard lock{reqThreadMutex};
			pending.test_and_set();
			genRequestsFront.push_back(request);
		}
	}
	
	template<class Self, class Layers, class SharedData>
	void Generator<Self, Layers, SharedData>::cleanCaches() {
		// TODO: Where to call this from?
		{
			// TOOD: note, global config is not thread safe.
			//Engine::getGlobalConfig().cvars.
			constexpr static uint64 cacheTargetThresholdBytes = 2ull * 1024 * 1024 * 1024; // TODO: should be cfg/cmd/console value.
			constexpr static uint64 cacheMaxThresholdBytes = 6ull * 1024 * 1024 * 1024; // TODO: should be cfg/cmd/console value.
			//constexpr static auto cacheTargetTimeout = Engine::Clock::Duration{std::chrono::minutes{2}}; // TODO: should be cfg/cmd/console value.
			constexpr static auto cacheTargetTimeout = Engine::Clock::Duration{std::chrono::seconds{20}}; // TODO: should be cfg/cmd/console value.
			uint64 totalBytes = 0;

			std::lock_guard lock{layerGenThreadMutex};
			Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
				totalBytes += layer.getCacheSizeBytes();
			});

			ENGINE_LOG2("cache bytes: {} / {} ({:.2f}%; {:.2f}%)({:.2f}GB)",
				totalBytes,
				cacheTargetThresholdBytes,
				totalBytes * (100 / static_cast<double>(cacheTargetThresholdBytes)),
				totalBytes * (100 / static_cast<double>(cacheMaxThresholdBytes)),
				totalBytes * (1.0 / (1 << 30))
			);

			if (totalBytes > cacheTargetThresholdBytes) {
				// Reduce the timeout based on how overcapacity the cache is. Down to zero at the max threshold.
				const auto t = totalBytes * (1.0 / cacheMaxThresholdBytes);
				const auto timeout = Engine::Math::lerp(cacheTargetTimeout, Engine::Clock::Duration{0}, std::min(1.0, t));
				const auto minAge = curSeq - static_cast<SeqNum>(timeout.count());

				Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
					layer.clearCache(minAge);
				});
			}
		}
	}

	template<class Self, class Layers, class SharedData>
	void Generator<Self, Layers, SharedData>::layerCoordinatorThread() {
		while (!allThreadsShouldExit.test()) {
			std::unique_lock lock{reqThreadMutex};

			ENGINE_DEBUG_ASSERT(genRequestsBack.empty(), "Any pending requests should have been processed and cleared at this point.");
			genRequestsFront.swap(genRequestsBack);

			if (genRequestsBack.empty()) {
				// At this point both the front and back request queues are empty and we
				// are not actively generating anything. We also have the request lock so
				// it's not possible any more requests have been pushed since swapping the
				// queues above. With that, it is safe to clear the pending flag.
				pending.clear();
				reqThreadWait.wait(lock);
			} else {
				lock.unlock();
				processGenRequests();
			}
		}
	}

	template<class Self, class Layers, class SharedData>
	void Generator<Self, Layers, SharedData>::processGenRequests() {
		for (const auto& genRequest : genRequestsBack) {
			this->request<Layer::BlendedBiomeBlock>(genRequest.chunkArea);
			this->request<Layer::BlendedBiomeStructures>(genRequest.chunkArea);
		}

		// nextSeq may or may not have been updated at this point. We just copy to curSeq
		// to avoid the need for an extra lock or frequent atomic lookup during generateLayers.
		curSeq = nextSeq;
		generateLayers();

		{
			// We need the lock on the terrain and not the generator because we have
			// access to the requests here. It would be nice if the generator had the lock
			// instead of the terrain (then we could move this external and completely
			// decouple them), but then it is more work on the caller's side since you
			// would need to track the requests and keep checking if they are done.
			//
			// This is more straight forward to implement and also more effecient, at the
			// cost of slight coupling between the terrain and generator.
			const auto lock = terrain.lock();

			// Copy the generator data to the terrain.
			for (const auto& genRequest : genRequestsBack) {
				for (auto chunkCoord = genRequest.chunkArea.min; chunkCoord.x < genRequest.chunkArea.max.x; ++chunkCoord.x) {
					for (chunkCoord.y = genRequest.chunkArea.min.y; chunkCoord.y < genRequest.chunkArea.max.y; ++chunkCoord.y) {
						const auto regionCoord = chunkToRegion(chunkCoord);
						auto& region = terrain.getRegion({genRequest.realmId, regionCoord});
						const auto regionIdx = chunkToRegionIndex(chunkCoord, regionCoord);
						auto& populated = region.populated[regionIdx.x][regionIdx.y];

						if (!populated) {
							// For each block in the chunk.
							// TODO: Once layers are done this should go away. Can just access the
							//       layerBlendedBiomeBlock directly*.
							region.chunkAt(regionIdx) = layerBlendedBiomeBlock.get(chunkCoord);
							populated = true;
						}
					}
				}

				// TODO: Should be part of layers and/or have a populated check.
				layerBlendedBiomeStructures.get(genRequest.chunkArea, self(), genRequest.realmId, terrain);
			}
		}

		genRequestsBack.clear();
		cleanCaches();
	}
}
