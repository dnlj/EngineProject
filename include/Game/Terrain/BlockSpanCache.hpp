#pragma once

#include <Game/Terrain/BlockSpanX.hpp>


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
			class RegionIteratorImpl {
				private:
					friend class BlockSpanCache;
					using CacheT = std::conditional_t<IsConst, const BlockSpanCache, BlockSpanCache>;
					using StoreItT = std::conditional_t<IsConst, Store::const_iterator, Store::iterator>;

					CacheT* cache; // ref
					RegionUnit regionCoord;
					ChunkUnit regionCoordMax; // const
					StoreItT data{};
					ChunkUnit chunkCoord = regionToChunk({regionCoord, 0}).x;
					BlockUnit chunkIndex = 0;
					ChunkUnit regionIndex = 0;
					BlockUnit blockCoord = chunkToBlock({chunkCoord, 0}).x;
					ENGINE_DEBUG_ONLY(int sanity = 0);
			
					RegionIteratorImpl(
						CacheT& cache,
						RegionSpanX area)
						: cache{&cache}
						, regionCoord{area.min}
						, regionCoordMax{area.max} {

						dataFromRegionCoord();
					}
			
				public:
					// TODO: Could this be simplified with AreaWalker?
					ENGINE_INLINE_REL RegionIteratorImpl& operator++() noexcept {
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

					// TODO: remove operator bool. Since the iterator isn't always created
					//       with a regionSpanX we would also need to track max
					//       block/chunk. This is only used in a few places anyways so we
					//       should be able to find a more stable solution. This will also
					//       allow us to remove regionCoordMax and save on a bunch of
					//       calcs at call sites.
					ENGINE_INLINE_REL operator bool() const noexcept { return regionCoord != regionCoordMax; }

					ENGINE_INLINE_REL auto& operator*() noexcept { return *data; }
					ENGINE_INLINE_REL BlockUnit getBlockCoord() const noexcept { return blockCoord; }
					ENGINE_INLINE_REL BlockUnit getChunkIndex() const noexcept { return chunkIndex; }
					ENGINE_INLINE_REL ChunkUnit getChunkCoord() const noexcept { return chunkCoord; }
					ENGINE_INLINE_REL BlockUnit getRegionIndex() const noexcept { return regionIndex; }

				private:
					void dataFromRegionCoord() {
						auto newData = cache->cache.find(regionCoord);
						ENGINE_DEBUG_ASSERT(newData != cache->cache.end());
						data = newData->second.begin();
					}
			};

		public: 
			using Iterator = RegionIteratorImpl<false>;
			using ConstIterator = RegionIteratorImpl<true>;

		private:
			Engine::FlatHashMap<RegionUnit, Store> cache{};

		public:
			BlockSpanCache() = default;
			BlockSpanCache(BlockSpanCache&&) = default;
			BlockSpanCache(const BlockSpanCache&) = delete;

			// TODO: cleanup/normalize these various walk functions. Most of these don't
			//       work correctly as walks since they don't have correct bool operators for
			//       end range.

			ENGINE_INLINE auto walk(RegionSpanX area) noexcept {
				return Iterator{*this, area};
			}

			ENGINE_INLINE auto walk(RegionSpanX area) const noexcept {
				return ConstIterator{*this, area};
			}
			
			ENGINE_INLINE Store& get(RegionUnit regionCoordX) noexcept {
				const auto found = cache.find(regionCoordX);
				ENGINE_DEBUG_ASSERT(found != cache.end());
				return found->second;
			}
			
			ENGINE_INLINE const Store& get(RegionUnit regionCoordX) const noexcept {
				return const_cast<BlockSpanCache*>(this)->get(regionCoordX);
			}

			ENGINE_INLINE auto walk(BlockSpanX area) const noexcept {
				// Need to add one to max because it is an _exclusive_ bound. Technically,
				// the +1 isn't needed if area.max happens to be at index 0 of a new
				// region, but its simpler to always add one.
				const auto maxRegion = chunkToRegion(blockToChunk({area.max, 0})).x + 1;
				const auto minRegion = chunkToRegion(blockToChunk({area.min, 0})).x;
				const auto minRegionBlock = chunkToBlock(regionToChunk({minRegion, 0})).x;
				const auto blockRegionOffset = area.min - minRegionBlock;

				// TODO: Calc with a closed-form expression instead of loop.
				ConstIterator result{*this, {minRegion, maxRegion}};
				for (auto i = blockRegionOffset; i > 0; --i) {
					++result;
				};

				return result;
			}

			ENGINE_INLINE auto walk(ChunkUnit chunkX) const noexcept {
				const auto regionCoordX = chunkToRegion({chunkX, 0}).x;
				const auto baseBlockCoord = chunkToBlock(regionToChunk({regionCoordX, 0})).x;
				const auto blockCoord = chunkToBlock({chunkX, 0}).x;
				const auto offset = blockCoord - baseBlockCoord;
				ENGINE_DEBUG_ASSERT(offset >= 0 && offset <= std::tuple_size_v<Store>);
				return get(regionCoordX).begin() + offset;
			}
			
			// TODO: remove, this is temp while fixing block psan cache to use regions.
			//       They should instead be using walk for effecient access.
			T& at(const BlockUnit x) noexcept {
				const auto region = chunkToRegion(blockToChunk({x, 0})).x;
				ENGINE_DEBUG_ASSERT(cache.contains(region));

				const auto regionOffset = region * chunksPerRegion * blocksPerChunk;
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
