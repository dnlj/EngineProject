#pragma once

// Game
#include <Game/Terrain/temp.hpp>
#include <Game/Terrain/BlockSpanX.hpp>


namespace Game::Terrain {
	// TODO: Doc, caches value for every block in a span. In increments of regions.
	template<class T>
	class BlockSpanCache {
		public:
			using Data = std::array<T, regionSize.x * chunkSize.x>;
			class Store {
				public:
					Data data;

					// TODO: Remove mutable? Its misleading and could be confusing. Will
					//       need to remove const from some layer get functions though.
					mutable SeqNum lastUsed;

					bool populated = false;
			};

			/**
			 * Iterates each block in the given span.
			 */
			/*template<bool IsConst>
			class RegionIteratorImpl {
				private:
					friend class BlockSpanCache;
					using CacheT = std::conditional_t<IsConst, const BlockSpanCache, BlockSpanCache>;
					using StoreItT = std::conditional_t<IsConst, typename Data::const_iterator, typename Data::iterator>;

					CacheT* cache; // ref
					RegionUnit regionCoord;
					ChunkUnit regionCoordMax; // const
					StoreItT data{};
					ChunkUnit chunkCoord = regionToChunk({regionCoord, 0}).x;
					BlockUnit chunkIndex = 0;
					ChunkUnit regionIndex = 0;
					BlockUnit blockCoord = chunkToBlock({chunkCoord, 0}).x;
					SeqNum curSeq;
					ENGINE_DEBUG_ONLY(int sanity = 0);
			
					RegionIteratorImpl(
						CacheT& cache,
						RegionSpanX area,
						SeqNum curSeq)
						: cache{&cache}
						, regionCoord{area.min}
						, regionCoordMax{area.max}
						, curSeq{curSeq} {
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
						newData->second.lastUsed = curSeq;
						data = newData->second.data.begin();
					}
			};

		public: 
			using Iterator = RegionIteratorImpl<false>;
			using ConstIterator = RegionIteratorImpl<true>;*/

		private:
			Engine::FlatHashMap<RegionUnit, Store> cache{};

		public:
			BlockSpanCache() = default;
			BlockSpanCache(BlockSpanCache&&) = default;
			BlockSpanCache(const BlockSpanCache&) = delete;

			// TODO: cleanup/normalize these various get/at/walk functions. Most of these don't
			//       work correctly as walks since they don't have correct bool operators for
			//       end range.

			ENGINE_INLINE const Data& get(RegionUnit regionCoordX, SeqNum curSeq) const noexcept {
				const auto found = cache.find(regionCoordX);
				ENGINE_DEBUG_ASSERT(found != cache.end());
				found->second.lastUsed = curSeq;
				return found->second.data;
			}

			ENGINE_INLINE auto walk(ChunkUnit chunkX, SeqNum curSeq) const noexcept {
				const auto regionCoordX = chunkToRegion({chunkX, 0}).x;
				const auto baseBlockCoord = chunkToBlock(regionToChunk({regionCoordX, 0})).x;
				const auto blockCoord = chunkToBlock({chunkX, 0}).x;
				const auto offset = blockCoord - baseBlockCoord;
				ENGINE_DEBUG_ASSERT(offset >= 0 && offset <= std::tuple_size_v<Data>);
				return get(regionCoordX, curSeq).begin() + offset;
			}
			
			// TODO: remove, this is temp while fixing block psan cache to use regions.
			//       They should instead be using walk for efficient access.
			ENGINE_INLINE const T& at(const BlockUnit x, SeqNum curSeq) const noexcept {
				const auto regionCoordX = chunkToRegion(blockToChunk({x, 0})).x;
				ENGINE_DEBUG_ASSERT(cache.contains(regionCoordX));

				const auto regionOffset = regionCoordX * chunksPerRegion * blocksPerChunk;
				return get(regionCoordX, curSeq).at(x - regionOffset);
			}

			ENGINE_INLINE_REL void reserve(const RegionSpanX area) noexcept {
				for (RegionUnit x = area.min; x < area.max; ++x) {
					cache.try_emplace(x);
				}
			}

			ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept {
				static_assert(std::is_trivially_destructible_v<T>, "Will need to account for sizes in getCacheSizeBytes if non-trivial type is used.");
				return cache.size() * sizeof(Store);
			}

			ENGINE_INLINE_REL void clearCache(SeqNum minAge) noexcept {
				const auto before = getCacheSizeBytes();

				for (auto it = cache.begin(); it != cache.end();) {
					if (it->second.lastUsed < minAge) {
						it = cache.erase(it);
					} else {
						++it;
					}
				}

				//
				//
				//
				// TODO: remove once confirmed working (and before above);
				//
				//
				//
				const auto after = getCacheSizeBytes();
				ENGINE_INFO2("BlockSpanCache::clearCache = {} - {} = {} ({:.2f}GB)", before, after, before - after, (before-after) * (1.0 / (1 << 30)));
			}

			ENGINE_INLINE bool isPopulated(RegionUnit regionCoordX, SeqNum curSeq) {
				const auto found = cache.find(regionCoordX);
				if (found == cache.end()) { return false; }
				found->second.lastUsed = curSeq;
				return found->second.populated;
			}

			ENGINE_INLINE void populate(RegionUnit regionCoordX, SeqNum curSeq, auto&& func) {
				const auto found = cache.find(regionCoordX);
				ENGINE_DEBUG_ASSERT(found != cache.end());
				found->second.lastUsed = curSeq;
				if (found->second.populated) { return; }

				found->second.populated = true;
				func(found->second.data);
			}
	};

	template<class T>
	ENGINE_INLINE inline void removeGeneratedPartitions(BlockSpanCache<T>& cache, SeqNum seqNum, std::vector<RegionUnit>& partitions) {
		std::erase_if(partitions, [&](const RegionUnit& regionCoordX){ return cache.isPopulated(regionCoordX, seqNum); });
	}
}
