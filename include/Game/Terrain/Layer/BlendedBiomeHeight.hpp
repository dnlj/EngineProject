#pragma once

// Game
#include <Game/Terrain/BiomeBlend.hpp>
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/BlockSpanCache.hpp>
#include <Game/Terrain/Layer/CachedLayer.hpp>


// h0 = broad, world-scale terrain height variations.
// h1 = biome specific height variations. h1 includes h0 as an input. h1 is
//      currently only used as part of an intermediate step and not stored
//      anywhere.
// h2 = final blended height between all influencing biomes.

namespace Game::Terrain::Layer {
	// The absolute weight of each biome. These are non-normalized.
	class BlendedBiomeHeight : public CachedLayer, public DependsOn<> {
		public:
			using Partition = RegionUnit;
			using Index = ChunkUnit;

		public: // TODO: private, currently public during transition to layers.
			BlockSpanCache<BlockUnit> cache;

		public:
			using CachedLayer::CachedLayer;

			void request(const Partition regionCoordX, TestGenerator& generator);
			ENGINE_INLINE void removeGenerated(std::vector<Partition>& partitions) { removeGeneratedPartitions(cache, getSeq(), partitions); }
			void generate(const Partition regionCoordX, TestGenerator& generator);
			[[nodiscard]] ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept { return cache.getCacheSizeBytes(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) clearCache(SeqNum minAge) noexcept { return cache.clearCache(minAge); }
			ENGINE_INLINE_REL [[nodiscard]] auto get(const ChunkUnit chunkCoordX) const noexcept { return cache.getChunk(chunkCoordX, getSeq()); }

		private:
			[[nodiscard]] BiomeBlend populate(BlockVec blockCoord, const TestGenerator& generator) const noexcept;
	};
}

