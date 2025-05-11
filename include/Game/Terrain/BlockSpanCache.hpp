#pragma once


namespace Game::Terrain {
	// TODO: Doc, caches value for every block in a span. In increments of regions.
	template<class T>
	class BlockSpanCache {
		public:
			using Store = std::array<BlockUnit, regionSize.x * chunkSize.x>;

			/**
			 * Iterates each block in the given span.
			 */
			template<bool IsConst>
			class IteratorImpl {
				private:
					friend class BlockSpanCache;
					using CacheT = std::conditional_t<IsConst, const BlockSpanCache, BlockSpanCache>;
					using StoreItT = std::conditional_t<IsConst, const Store, Store>::iterator;

					CacheT& cache;
					RegionUnit regionCoord;
					const ChunkUnit regionCoordMax;
					StoreItT data{};
					ChunkUnit chunkCoord = regionToChunk({regionCoord, 0}).x;
					BlockUnit chunkIndex = 0;
					ChunkUnit regionIndex = 0;
					BlockUnit blockCoord = chunkToBlock({chunkCoord, 0}).x;
					ENGINE_DEBUG_ONLY(int sanity = 0);
			
					IteratorImpl(
						CacheT& cache,
						RegionSpanX area)
						: cache{cache}
						, regionCoord{area.min}
						, regionCoordMax{area.max} {

						dataFromRegionCoord();
					}
			
				public:
					ENGINE_INLINE_REL IteratorImpl& operator++() noexcept {
						ENGINE_DEBUG_ASSERT(regionCoord != regionCoordMax);

						ENGINE_DEBUG_ONLY(++sanity);
						ENGINE_DEBUG_ASSERT(sanity < 10'000);

						++blockCoord;
			
						++chunkIndex;
						if (chunkIndex == chunkSize.x) {
							++chunkCoord;
							chunkIndex = 0;
			
							++regionIndex;
							if (regionIndex == regionSize.x) {
								++regionCoord;
								regionIndex = 0;

								if (regionCoord != regionCoordMax) {
									dataFromRegionCoord();
								} else {
									// We need this case because it is valid to increment _to_ the
									// end iterator, just not use or increment it.
									//data = nullptr;
									data = {};
								}

								// Return early to avoid incrementing the store below.
								return *this;
							}
						}

						// At this point the region has not changed. We can just increment
						// the current data.
						++data;
						return *this;
					}
			
					ENGINE_INLINE_REL auto& operator*() noexcept { return *data; }
					ENGINE_INLINE_REL operator bool() const noexcept { return regionCoord != regionCoordMax; }
					ENGINE_INLINE_REL BlockUnit getBlockCoord() const noexcept { return blockCoord; }
					ENGINE_INLINE_REL BlockUnit getChunkIndex() const noexcept { return chunkIndex; }
					ENGINE_INLINE_REL ChunkUnit getChunkCoord() const noexcept { return chunkCoord; }
					ENGINE_INLINE_REL BlockUnit getRegionIndex() const noexcept { return regionIndex; }

				private:
					void dataFromRegionCoord() {
						auto newData = cache.cache.find(regionCoord);
						ENGINE_DEBUG_ASSERT(newData != cache.cache.end());
						data = newData->second.begin();
					}
			};

		public: 
			using Iterator = IteratorImpl<false>;
			using ConstIterator = IteratorImpl<true>;

		private:
			Engine::FlatHashMap<RegionUnit, Store> cache{};

		public:
			BlockSpanCache() = default;
			BlockSpanCache(BlockSpanCache&&) = default;
			BlockSpanCache(const BlockSpanCache&) = delete;

			ENGINE_INLINE auto walk(RegionSpanX area) noexcept {
				return Iterator{*this, area};
			}

			ENGINE_INLINE auto walk(RegionSpanX area) const noexcept {
				return ConstIterator{*this, area};
			}
			
			// TODO: remove, this is temp while fixing block psan cache to use regions.
			T& at(const BlockUnit x) noexcept {
				auto const region = chunkToRegion(blockToChunk({x, 0})).x;
				ENGINE_DEBUG_ASSERT(cache.contains(region));

				auto const regionOffset = region * chunksPerRegion * blocksPerChunk;
				return cache.at(region).at(x - regionOffset);
			}

			// TODO: remove, this is temp while fixing block psan cache to use regions.
			const T& at(const BlockUnit x) const noexcept {
				return const_cast<BlockSpanCache&>(*this).at(x);
			}

			ENGINE_INLINE_REL void reserve(const RegionSpanX area) noexcept {
				for (RegionUnit x = area.min; x < area.max; ++x) {
					cache.try_emplace(x);
				}
			}

			// TODO: Should these each take a block/chunk/region span instead of one as a whole?
			//// TODO: Function sig concept
			//ENGINE_INLINE void forEachBlock(const RegionSpanX area, auto&& func) {
			//	const auto min = area.min * chunksPerRegion * blocksPerChunk;
			//	const auto max = area.max * chunksPerRegion * blocksPerChunk;
			//	for (auto x = min; x < max; ++x) { func(x, at(x)); }
			//}
			//
			//// TODO: Function sig concept
			//ENGINE_INLINE void forEachChunk(const RegionSpanX area, auto&& func) {
			//	const auto min = area.min * chunksPerRegion;
			//	const auto max = area.max * chunksPerRegion;
			//	for (auto x = min; x < max; ++x) { func(x, at(x)); }
			//}
			//
			//ENGINE_INLINE void forEachRegion(const RegionSpanX area, auto&& func) {
			//	const auto min = area.min;
			//	const auto max = area.max;
			//	for (auto x = min; x < max; ++x) { func(x, at(x)); }
			//}
	};
}
