#pragma once

// Currently only used by BlendedBiomeStructureInfo so inline is preferable.
/*
// Engine
#include <Engine/StaticVector.hpp>

// Game
#include <Game/Terrain/Layer/CachedLayer.hpp>
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/RegionDataCache.hpp>
#include <Game/Terrain/terrain.hpp>
#include <Game/universal.hpp>


namespace Game::Terrain::Layer {
	class ChunkBiomeContributions : public CachedLayer, public DependsOn<> {
		public:
			using Partition = UniversalChunkCoord;
			using Index = Partition;

		private:
			using BiomeContributions = Engine::StaticVector<BiomeId, 4>;
			RegionDataCache<BiomeContributions> cache;

		public:
			using CachedLayer::CachedLayer;

			void request(const Partition chunkCoord, TestGenerator& generator);
			void generate(const Partition chunkCoord, TestGenerator& generator);
			ENGINE_INLINE [[nodiscard]] const BiomeContributions& get(const Index chunkCoord) const noexcept;

			ENGINE_INLINE void removeGenerated(std::vector<Partition>& partitions) { removeGeneratedPartitions(cache, getSeq(), partitions); }
			[[nodiscard]] ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept { return cache.getCacheSizeBytes(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) clearCache(SeqNum minAge) noexcept { return cache.clearCache(minAge); }
	};
}
*/
