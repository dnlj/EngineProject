#pragma once

// Game
#include <Game/Terrain/BiomeBlend.hpp>
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/ChunkDataCache.hpp>
#include <Game/Terrain/Layer/CachedLayer.hpp>


namespace Game::Terrain::Layer {
	class BlendedBiomeWeights;

	class BlendedBiomeBasis : public CachedLayer, public DependsOn<BlendedBiomeWeights> {
		public:
			using Partition = ChunkVec;
			using Index = ChunkVec;

		private:
			ChunkDataCache<BasisInfo> cache;

		public:
			using CachedLayer::CachedLayer;

			void request(const Partition chunkCoord, TestGenerator& generator);
			ENGINE_INLINE void removeGenerated(std::vector<Partition>& partitions) { removeGeneratedPartitions(cache, getSeq(), partitions); }
			void generate(const Partition chunkCoord, TestGenerator& generator);
			[[nodiscard]] const ChunkStore<BasisInfo>& get(const Index chunkCoord) const noexcept;
			[[nodiscard]] ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept { return cache.getCacheSizeBytes(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) clearCache(SeqNum minAge) noexcept { return cache.clearCache(minAge); }

		private:
			[[nodiscard]] BasisInfo populate(const BlockVec blockCoord, const BlockUnit h2, const BiomeBlend& blend, const TestGenerator& generator) const noexcept;
	};
}
