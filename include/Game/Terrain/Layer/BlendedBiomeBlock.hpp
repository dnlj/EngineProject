#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/ChunkArea.hpp>
#include <Game/Terrain/Layer/CachedLayer.hpp>
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/RegionDataCache.hpp>


namespace Game::Terrain::Layer {
	class BlendedBiomeBlock : public CachedLayer, public DependsOn<> {
		public:
			using Partition = ChunkVec;
			using Index = ChunkVec;

		private:
			RegionDataCache<MapChunk> cache;

		public:
			using CachedLayer::CachedLayer;

			void request(const Partition chunkCoord, TestGenerator& generator);
			ENGINE_INLINE void removeGenerated(std::vector<Partition>& partitions) { removeGeneratedPartitions(cache, getSeq(), partitions); }
			void generate(const Partition chunkCoord, TestGenerator& generator);
			[[nodiscard]] const MapChunk& get(const Index chunkCoord) const noexcept;
			[[nodiscard]] ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept { return cache.getCacheSizeBytes(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) clearCache(SeqNum minAge) noexcept { return cache.clearCache(minAge); }

		private:
			// TODO: no reason populate to be a member function on most (all) layers. Move to plain function in the cpp file.
			[[nodiscard]] BlockId populate(const BlockVec blockCoord, const BasisInfo& basisInfo, const TestGenerator& generator) const noexcept;
	};
}
