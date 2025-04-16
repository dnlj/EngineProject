#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	class BiomeWeights;

	// The biome weights for a given area.
	class BiomeBlended : public DependsOn<BiomeWeights> {
		public:
			using Range = ChunkArea;
			using Index = ChunkVec;

		private:
			ChunkCache<BiomeBlend> cache;

		public:
			void request(const Range area, TestGenerator& generator);
			void generate(const Range area, TestGenerator& generator);
			[[nodiscard]] const ChunkStore<BiomeBlend>& get(const Index chunkCoord) const noexcept;

		private:
			[[nodiscard]] BiomeBlend populate(const BlockVec blockCoord, BiomeBlend blend, const TestGenerator& generator) const noexcept;
	};
}

