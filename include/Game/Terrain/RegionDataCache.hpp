#pragma once

// Game
#include <Game/Terrain/RegionStore.hpp>


namespace Game::Terrain {
	/**
	 * Store data per chunk for each region.
	 */
	template<class ChunkData>
	class RegionDataCache {
		private:
			using Store = RegionStore<ChunkData>;
			Engine::FlatHashMap<RegionVec, std::unique_ptr<Store>> regions;

		public:
			RegionDataCache() = default;
			RegionDataCache(RegionDataCache&&) = default;
			RegionDataCache(const RegionDataCache&) = delete;

			ENGINE_INLINE Store& at(RegionVec regionCoord) noexcept {
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end(), "Attempting to access region outside of RegionDataCache.");
				return *found->second;
			}

			ENGINE_INLINE const Store& at(RegionVec regionCoord) const noexcept {
				return const_cast<RegionDataCache*>(this)->at(regionCoord);
			}

			ENGINE_INLINE void reserve(RegionVec regionCoord) noexcept {
				const auto found = regions.find(regionCoord);
				if (found == regions.end()) {
					regions.try_emplace(regionCoord, std::make_unique<Store>());
				}
			}

			ENGINE_INLINE void reserve(RegionArea regionArea) noexcept {
				for (auto x = regionArea.min.x; x < regionArea.max.x; ++x) {
					for (auto y = regionArea.min.y; y < regionArea.max.y; ++y) {
						reserve({x, y});
					}
				}
			}

			ENGINE_INLINE_REL decltype(auto) populate(const ChunkVec chunkCoord, auto&& func) {
				const auto regionCoord = chunkToRegion(chunkCoord);
				const auto regionIndex = chunkToRegionIndex(chunkCoord, regionCoord);
				auto& regionStore = this->at(regionCoord);
				if (regionStore.isPopulated(regionIndex)) { return; }

				regionStore.setPopulated(regionIndex);
				auto& chunkStore = regionStore.at(regionIndex);
				return func(chunkStore);
			}

			ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept {
				static_assert(std::is_trivially_destructible_v<ChunkData>, "Will need to account for sizes in getCacheSizeBytes if non-trivial type is used.");
				return regions.size() * sizeof(Store);
			}
	};
}
