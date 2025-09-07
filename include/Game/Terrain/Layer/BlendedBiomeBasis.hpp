#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/ChunkDataCache.hpp>
#include <Game/Terrain/Layer/CachedLayer.hpp>


namespace Game::Terrain::Layer {
	class BlendedBiomeWeights;

	class BlendedBiomeBasis : public CachedLayer, public DependsOn<BlendedBiomeWeights> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkVec;
			using Index = ChunkVec;

		private:
			ChunkDataCache<BasisInfo> cache;

		public:
			using CachedLayer::CachedLayer;

			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void partition(std::vector<Range>& requests, std::vector<Partition>& partitions) { flattenRequests(requests, partitions); }
			void generate(const Partition chunkCoord, TestGenerator& generator);
			[[nodiscard]] const ChunkStore<BasisInfo>& get(const Index chunkCoord) const noexcept;
			[[nodiscard]] ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept { return cache.getCacheSizeBytes(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) clearCache(SeqNum minAge) noexcept { return cache.clearCache(minAge); }

		private:
			[[nodiscard]] BasisInfo populate(const BlockVec blockCoord, const BiomeBlend& blend, const TestGenerator& generator) const noexcept;
	};
}
