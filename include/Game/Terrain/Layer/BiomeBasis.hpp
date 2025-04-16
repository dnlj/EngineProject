#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	class BiomeBlended;

	class BiomeBasis : public DependsOn<BiomeBlended> {
		public:
			using Range = ChunkArea;
			using Index = ChunkVec;

		private:
			ChunkCache<BasisInfo> cache;

		public:
			void request(const Range area, TestGenerator& generator);
			void generate(const Range area, TestGenerator& generator);
			[[nodiscard]] const ChunkStore<BasisInfo>& get(const Index chunkCoord) const noexcept;

		private:
			[[nodiscard]] BasisInfo populate(const BlockVec blockCoord, const BiomeBlend& blend, const TestGenerator& generator) const noexcept;
	};
}
