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


		// Rough layer overview. In general any "blended" layer is one that deals with the influence
		// of multiple biomes (BiomeWeights).
		// 
		// - Determine the ground level (WorldBaseHeight).
		// - Determine the biome influence (RawBiome, RawBiomeWeights).
		// - Determine the adjusted primary biome (BlendedBiomeWeights).
		// - Determine the biome adjusted ground level (BlendedBiomeHeight).
		// - Determine basis/density (if solid or air) (BlendedBiomeBasis).
		// - Determine the natrual block if solid (BlendedBiomeBlock).
		// - Determine possible structures (BlendedBiomeStructureInfo).
		// - Modify the terrain and add entities for any valid/non-overlapping structures (BlendedBiomeStructures).

		{
			std::lock_guard lock{reqThreadMutex};
			pending.test_and_set();
			genRequestsFront.push_back(request);
		}
	}
	
	template<class Self, class Layers, class SharedData>
	void Generator<Self, Layers, SharedData>::cleanCaches() {
		uint64 totalBytes = 0;

		// We do not need to lock layerGenThreadMutex here because cleanCaches is only called
		// synchronously from the coordinator thread.
		Engine::forEach(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
			totalBytes += layer.getCacheSizeBytes();
		});

		// Don't spam the server console with constant cleanup messages.
		if constexpr (ENGINE_CLIENT) {
			ENGINE_LOG2("Generation Cache: {} / {} ({:.2f}%; {:.2f}%)({:.2f}GB)",
				totalBytes,
				cacheTargetThresholdBytes,
				totalBytes * (100 / static_cast<double>(cacheTargetThresholdBytes)),
				totalBytes * (100 / static_cast<double>(cacheMaxThresholdBytes)),
				totalBytes * (1.0 / (1 << 30))
			);
		}

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
		// We clean caches at the start instead of the end to avoid issues with the TerrainPreview
		// when generating large amounts of terrain, which fills the entire cache, and then trying
		// to access layers which have already been cleaned. Moving it to the start instead of end
		// avoids this by deferring the cleanup until the next call so the layers are not cleaned
		// immediately.
		//
		// From the gameplay side this should be fine since we are queueing requests all the time so
		// that cache will be cleaned either way.
		cleanCaches();
		currentLayer = layerIdEnd();

		{
			// Avoid regenerating already generated data. Even with caches data from immediate
			// layers would be regenerated without this. We would also regenerate data that timed
			// out of the cache that is already loaded on the terrain.
			const auto lock = terrain.lock();

			for (const auto& genRequest : genRequestsBack) {
				genRequest.chunkArea.forEach([&](const auto& chunkCoord) {
					// TODO: It might be beneficial to create a struct that contains all of
					//       region/chunk/block coord+indexes and calculate that upfront since basically
					//       every system does those conversions.
					const UniversalChunkCoord uniChunkCoord = {genRequest.realmId, chunkCoord};
					if (!terrain.isChunkLoaded(uniChunkCoord)) {
						this->request<Layer::BlendedBiomeBlock>(uniChunkCoord);
						this->request<Layer::BlendedBiomeStructures>(uniChunkCoord);
					}
				});
			}
		}

		// It is important we use forEachReverse and not forEach so that the _most_
		// dependent layers are evaluated first.
		Engine::forEachReverse(layers, [&]<class Layer>(Layer& layer) ENGINE_INLINE_REL {
			currentLayer = layerId<Layer>();
			processRequests<Layer>();
		});

		ENGINE_DEBUG_ASSERT(currentLayer == 0);
		currentLayer = layerIdEnd();

		// nextSeq may or may not have been updated at this point. We just copy to curSeq
		// to avoid the need for an extra lock or frequent atomic lookup during generateLayers.
		curSeq = nextSeq; // TODO: shouldn't we be doing this at the start of the function?

		generateLayers();

		{
			// TODO: totalBlendedBiomeBlockRequests and totalBlendedBiomeStructuresRequests likely
			//       contain a lot of duplicates at this point. Try deduplicating or using a set to se how
			//       that affects things.
			
			// We need the lock on the terrain and not the generator because we have
			// access to the requests here. It would be nice if the generator had the lock
			// instead of the terrain (then we could move this external and completely
			// decouple them), but then it is more work on the caller's side since you
			// would need to track the requests and keep checking if they are done.
			//
			// This is more straight forward to implement and also more efficient, at the
			// cost of slight coupling between the terrain and generator.
			const auto lock = terrain.lock();

			// Copy the block data to the terrain.
			// We need two	 loops. One for block data and one for structure data. This is needed
			// since generating structures may rely on the terrain data and could end up
			// reading/writing structure data before that chunk is generated otherwise. If you try
			// combining them you will noticed that _some_ (not all) structures that span chunk
			// boundries may end up cut off. Which ones are cut off depends on the generation order.
			for (const auto& chunkCoord : totalBlendedBiomeBlockRequests) {
				const auto regionCoord = chunkCoord.toRegion();
				auto& region = terrain.getRegion(regionCoord);
				const auto regionIdx = chunkCoord.toRegionIndex(regionCoord);
				auto& chunkStage = region.populated[regionIdx.x][regionIdx.y];

				if (chunkStage == ChunkStage::Uninitialized) {
					region.chunkAt(regionIdx) = layerBlendedBiomeBlock.get(chunkCoord);
					chunkStage = ChunkStage::TerrainComplete;
				}
			}

			// Copy the structure data to the terrain.
			for (const auto& chunkCoord : totalBlendedBiomeStructuresRequests) {
				const auto regionCoord = chunkToRegion(chunkCoord.pos);
				auto& region = terrain.getRegion({chunkCoord.realmId, regionCoord});
				const auto regionIdx = chunkToRegionIndex(chunkCoord.pos, regionCoord);
				auto& chunkStage = region.populated[regionIdx.x][regionIdx.y];

				if (chunkStage == ChunkStage::TerrainComplete) {
					layerBlendedBiomeStructures.get(chunkCoord, self(), terrain);
					chunkStage = ChunkStage::StructuresComplete;
				}
			}
		}

		genRequestsBack.clear();
	}
}
