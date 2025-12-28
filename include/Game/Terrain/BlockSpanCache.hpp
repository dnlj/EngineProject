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

		private:
			Engine::FlatHashMap<UniversalRegionCoordX, Store> cache{};

		public:
			BlockSpanCache() = default;
			BlockSpanCache(BlockSpanCache&&) = default;
			BlockSpanCache(const BlockSpanCache&) = delete;

			// TODO: Rename `get` once we have strong typedefs for overload.
			ENGINE_INLINE_REL const Data& getRegion(UniversalRegionCoordX regionCoordX, const SeqNum curSeq) const noexcept {
				const auto found = cache.find(regionCoordX);
				ENGINE_DEBUG_ASSERT(found != cache.end(), "Attempting to get uninitialized region.");
				found->second.lastUsed = curSeq;
				return found->second.data;
			}

			// TODO: Rename `get` once we have strong typedefs for overload.
			ENGINE_INLINE_REL auto getChunk(const UniversalChunkCoordX chunkCoordX, const SeqNum curSeq) const noexcept {
				const auto regionCoordX = chunkCoordX.toRegion();
				const auto offsetBlocks = chunkSize.x * (chunkCoordX.pos - regionCoordX.toChunk().pos);
				ENGINE_DEBUG_ASSERT(offsetBlocks >= 0 && offsetBlocks <= std::tuple_size_v<Data>);
				return getRegion(regionCoordX, curSeq).begin() + offsetBlocks;
			}

			ENGINE_INLINE_REL void reserve(const UniversalRegionCoordX regionCoordX) noexcept {
				cache.try_emplace(regionCoordX);
			}

			ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept {
				static_assert(std::is_trivially_destructible_v<T>, "Will need to account for sizes in getCacheSizeBytes if non-trivial type is used.");
				return cache.size() * sizeof(Store);
			}

			ENGINE_INLINE_REL void clearCache(SeqNum minAge) noexcept {
				//const auto before = getCacheSizeBytes();

				for (auto it = cache.begin(); it != cache.end();) {
					if (it->second.lastUsed < minAge) {
						it = cache.erase(it);
					} else {
						++it;
					}
				}

				//const auto after = getCacheSizeBytes();
				//ENGINE_INFO2("BlockSpanCache::clearCache = {} - {} = {} ({:.2f}GB)", before, after, before - after, (before-after) * (1.0 / (1 << 30)));
			}

			ENGINE_INLINE bool isPopulated(UniversalRegionCoordX regionCoordX, const SeqNum curSeq) {
				const auto found = cache.find(regionCoordX);
				if (found == cache.end()) { return false; }
				found->second.lastUsed = curSeq;
				return found->second.populated;
			}

			ENGINE_INLINE void populate(UniversalRegionCoordX regionCoordX, const SeqNum curSeq, auto&& func) {
				const auto found = cache.find(regionCoordX);
				ENGINE_DEBUG_ASSERT(found != cache.end());
				found->second.lastUsed = curSeq;
				if (found->second.populated) { return; }

				found->second.populated = true;
				func(found->second.data);
			}
	};

	template<class T>
	ENGINE_INLINE inline void removeGeneratedPartitions(BlockSpanCache<T>& cache, const SeqNum curSeq, std::vector<UniversalRegionCoordX>& partitions) {
		std::erase_if(partitions, [&](const UniversalRegionCoordX& regionCoordX){ return cache.isPopulated(regionCoordX, curSeq); });
	}
}
