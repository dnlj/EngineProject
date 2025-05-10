#pragma once


namespace Game::Terrain {
	/**
	 * Stores, potentially partially populated, data for every chunk in a region.
	 */
	template<class T>
	class RegionStore {
		private:
			T store[regionSize.x][regionSize.y]{};
			bool populated[regionSize.x][regionSize.y]{};

		public:
			RegionStore() = default;
			RegionStore(RegionStore&&) = default;
			RegionStore(const RegionStore&) = delete;

			ENGINE_INLINE_REL bool isPopulated(RegionVec regionIndex) const noexcept {
				ENGINE_DEBUG_ASSERT((regionIndex.x >= 0) && (regionIndex.x < regionSize.x), "Attempting to access chunk outside of RegionStore.");
				ENGINE_DEBUG_ASSERT((regionIndex.y >= 0) && (regionIndex.y < regionSize.y), "Attempting to access chunk outside of RegionStore.");
				return populated[regionIndex.x][regionIndex.y];
			}

			ENGINE_INLINE_REL void setPopulated(RegionVec regionIndex) noexcept {
				ENGINE_DEBUG_ASSERT((regionIndex.x >= 0) && (regionIndex.x < regionSize.x), "Attempting to access chunk outside of RegionStore.");
				ENGINE_DEBUG_ASSERT((regionIndex.y >= 0) && (regionIndex.y < regionSize.y), "Attempting to access chunk outside of RegionStore.");
				populated[regionIndex.x][regionIndex.y] = true;
			}

			ENGINE_INLINE_REL T& at(RegionVec regionIndex) noexcept {
				ENGINE_DEBUG_ASSERT((regionIndex.x >= 0) && (regionIndex.x < regionSize.x), "Attempting to access chunk outside of RegionStore.");
				ENGINE_DEBUG_ASSERT((regionIndex.y >= 0) && (regionIndex.y < regionSize.y), "Attempting to access chunk outside of RegionStore.");
				return store[regionIndex.x][regionIndex.y];
			}

			ENGINE_INLINE_REL const T& at(RegionVec regionIndex) const noexcept {
				ENGINE_DEBUG_ASSERT(isPopulated(regionIndex), "Attempting to access unpopulated chunk.");
				return const_cast<RegionStore*>(this)->at(regionIndex);
			}
	};
}
