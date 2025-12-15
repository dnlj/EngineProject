#pragma once

// Game
#include <Game/Terrain/BiomeBlend.hpp>
#include <Game/Terrain/ChunkDataCache.hpp>
#include <Game/Terrain/Layer/CachedLayer.hpp>


namespace Game::Terrain::Layer {
	// The absolute weight of each biome. These are non-normalized.
	class RawBiomeWeights : public CachedLayer, public DependsOn<> {
		public:
			using Partition = ChunkVec;
			using Index = ChunkVec;

		private:
			ChunkDataCache<BiomeBlend> cache;

		public:
			using CachedLayer::CachedLayer;

			void request(const Partition chunkCoord, TestGenerator& generator);
			ENGINE_INLINE void removeGenerated(std::vector<Partition>& partitions) { removeGeneratedPartitions(cache, getSeq(), partitions); }
			void generate(const Partition chunkCoord, TestGenerator& generator);
			[[nodiscard]] const ChunkStore<BiomeBlend>& get(const Index chunkCoord) const noexcept;
			[[nodiscard]] ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept { return cache.getCacheSizeBytes(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) clearCache(SeqNum minAge) noexcept { return cache.clearCache(minAge); }


		private:
			// TODO: create an example layer with notes such as:
			// 
			//       Populate functions should prefer to dependencies as arguments if possible. This makes dependencies obvious and keeps
			//       them in one place. This also makes some more optimizations
			//       possible/obvious such as lifting dependencies out of loops.
			/**
			 * Generate the value at a single index.
			 */
			[[nodiscard]] BiomeBlend populate(BlockVec blockCoord, const TestGenerator& generator) const noexcept;
	};
}

