#pragma once

// Game
#include <Game/Terrain/BiomeBlend.hpp>
#include <Game/Terrain/Layer/CachedLayer.hpp>
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/ChunkDataCache.hpp>


namespace Game::Terrain::Layer {
	// The biome weights for a given area.
	class BlendedBiomeWeights : public CachedLayer, public DependsOn<> {
		public:
			using Partition = UniversalChunkCoord;
			using Index = UniversalChunkCoord;

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
			[[nodiscard]] BiomeBlend populate(const UniversalBlockCoord blockCoord, BiomeBlend blend, const TestGenerator& generator) const noexcept;
	};
}

