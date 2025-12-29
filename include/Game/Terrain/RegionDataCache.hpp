#pragma once

// Game
#include <Game/Terrain/RegionStore.hpp>
#include <Game/universal.hpp>


namespace Game::Terrain {
	/**
	 * Store data per chunk for each region.
	 */
	template<class ChunkData>
	class RegionDataCache {
		private:
			using Store = RegionStore<ChunkData>;
			Engine::FlatHashMap<UniversalRegionCoord, std::unique_ptr<Store>> regions;

		public:
			RegionDataCache() = default;
			RegionDataCache(RegionDataCache&&) = default;
			RegionDataCache(const RegionDataCache&) = delete;

			ENGINE_INLINE Store& at(UniversalRegionCoord regionCoord, SeqNum curSeq) noexcept {
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end(), "Attempting to access region outside of RegionDataCache.");
				found->second->lastUsed = curSeq;
				return *found->second;
			}

			ENGINE_INLINE const Store& at(UniversalRegionCoord regionCoord, SeqNum curSeq) const noexcept {
				return const_cast<RegionDataCache*>(this)->at(regionCoord, curSeq);
			}

			// TODO: rename to just `reserve` once strong typdefs are in place for ChunkVec/RegionVec.
			ENGINE_INLINE void reserveRegion(UniversalRegionCoord regionCoord, SeqNum curSeq) noexcept {
				auto found = regions.find(regionCoord);
				if (found == regions.end()) {
					found = regions.try_emplace(regionCoord, std::make_unique<Store>()).first;
				}

				found->second->lastUsed = curSeq;
			}
			
			ENGINE_INLINE bool isPopulated(const UniversalChunkCoord chunkCoord, SeqNum curSeq) {
				const auto regionCoord = chunkCoord.toRegion();
				const auto regionIndex = chunkCoord.toRegionIndex(regionCoord);
				auto& regionStore = this->at(regionCoord, curSeq);
				return regionStore.isPopulated(regionIndex);
			}

			ENGINE_INLINE_REL decltype(auto) populate(const UniversalChunkCoord chunkCoord, SeqNum curSeq, auto&& func) {
				const auto regionCoord = chunkCoord.toRegion();
				const auto regionIndex = chunkCoord.toRegionIndex(regionCoord);
				auto& regionStore = this->at(regionCoord, curSeq);
				if (regionStore.isPopulated(regionIndex)) { return; }

				regionStore.setPopulated(regionIndex);
				auto& chunkStore = regionStore.at(regionIndex);
				return func(chunkStore);
			}

			ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept {
				static_assert(std::is_trivially_destructible_v<ChunkData>, "Will need to account for sizes in getCacheSizeBytes if non-trivial type is used.");
				return regions.size() * sizeof(Store);
			}

			ENGINE_INLINE_REL void clearCache(SeqNum minAge) noexcept {
				//const auto before = getCacheSizeBytes();

				for (auto it = regions.begin(); it != regions.end();) {
					if (it->second->lastUsed < minAge) {
						it = regions.erase(it);
					} else {
						++it;
					}
				}

				//const auto after = getCacheSizeBytes();
				//ENGINE_INFO2("RegionDataCache::clearCache = {} - {} = {} ({:.2f}GB)", before, after, before - after, (before-after) * (1.0 / (1 << 30)));
			}
	};

	template<class T>
	ENGINE_INLINE inline void removeGeneratedPartitions(RegionDataCache<T>& cache, SeqNum seqNum, std::vector<UniversalChunkCoord>& partitions) {
		std::erase_if(partitions, [&](const UniversalChunkCoord& chunkCoord){ return cache.isPopulated(chunkCoord, seqNum); });
	}
}
