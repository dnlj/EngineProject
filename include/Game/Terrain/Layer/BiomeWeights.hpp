#pragma once

#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.

namespace Game::Terrain::Layer {
	class BiomeRaw;

	// The absolute weight of each biome. These are non-normalized.
	class BiomeWeights : public DependsOn<BiomeRaw> {
		public:
			using Range = ChunkArea;
			using Index = ChunkVec;

		private:
			ChunkAreaCache<BiomeBlend> cache;

		public:
			void request(const Range area, TestGenerator& generator);
			void generate(const Range area, TestGenerator& generator);
			[[nodiscard]] const ChunkStore<BiomeBlend>& get(const Index chunkCoord) const noexcept;

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

