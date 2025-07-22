#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/DependsOn.hpp>


namespace Game::Terrain::Layer {
	class BiomeWeights;

	// The biome weights for a given area.
	class BiomeBlended : public DependsOn<BiomeWeights> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkArea;
			using Index = ChunkVec;

		private:
			ChunkDataCache<BiomeBlend> cache;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void partition(std::vector<Range>& requests, std::vector<Partition>& partitions) { partitions = std::move(requests); }
			void generate(const Range area, TestGenerator& generator);
			[[nodiscard]] const ChunkStore<BiomeBlend>& get(const Index chunkCoord) const noexcept;

		private:
			[[nodiscard]] BiomeBlend populate(const BlockVec blockCoord, BiomeBlend blend, const TestGenerator& generator) const noexcept;
	};
}

