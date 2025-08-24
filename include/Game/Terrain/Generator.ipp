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

		//// TODO: avoid name conflict with arguments `request`
		//this->request<Layer::BiomeBlock>(request.chunkArea);
		//this->request<Layer::BiomeStructures>(request.chunkArea);
		//generateLayers();
		//
		//// Copy the generator data to the terrain.
		//for (auto chunkCoord = request.chunkArea.min; chunkCoord.x < request.chunkArea.max.x; ++chunkCoord.x) {
		//	for (chunkCoord.y = request.chunkArea.min.y; chunkCoord.y < request.chunkArea.max.y; ++chunkCoord.y) {
		//		const auto regionCoord = chunkToRegion(chunkCoord);
		//		auto& region = terrain.getRegion({request.realmId, regionCoord});
		//		const auto regionIdx = chunkToRegionIndex(chunkCoord, regionCoord);
		//		auto& populated = region.populated[regionIdx.x][regionIdx.y];
		//		
		//		if (!populated) {
		//			// For each block in the chunk.
		//			// TODO: Once layers are done this should go away. Can just access the
		//			//       layerBiomeBlock directly.
		//			const auto& chunkBiomeBlock = layerBiomeBlock.get(chunkCoord);
		//			auto& chunk = region.chunkAt(regionIdx);
		//			chunk = chunkBiomeBlock;
		//			populated = true;
		//		}
		//	}
		//}
		//
		//// TODO: Should be part of layers and/or have a populated check.
		//layerBiomeStructures.get(request.chunkArea, self(), request.realmId, terrain);


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
		// TODO: doesn't genRequests need a mutext? Should have two queues and swap to avodi blocking.
		for (const auto& genRequest : genRequestsBack) {
			this->request<Layer::BiomeBlock>(genRequest.chunkArea);
			this->request<Layer::BiomeStructures>(genRequest.chunkArea);
		}

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
							//       layerBiomeBlock directly.
							const auto& chunkBiomeBlock = layerBiomeBlock.get(chunkCoord);
							auto& chunk = region.chunkAt(regionIdx);
							chunk = chunkBiomeBlock;
							populated = true;
						}
					}
				}

				// TODO: Should be part of layers and/or have a populated check.
				layerBiomeStructures.get(genRequest.chunkArea, self(), genRequest.realmId, terrain);
			}
		}

		genRequestsBack.clear();
	}
}
