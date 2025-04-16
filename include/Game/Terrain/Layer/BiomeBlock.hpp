#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	class BiomeBlock : public DependsOn<> {
		public:
			using Range = ChunkArea;
			using Index = ChunkVec;

		private:
			RegionCache<MapChunk> cache;

		public:
			void request(const Range area, TestGenerator& generator);
			void generate(const Range area, TestGenerator& generator);
			[[nodiscard]] const MapChunk& get(const Index chunkCoord) const noexcept;

		private:
			// TODO: no reason populate to be a member funciton on most (all) layers. Move to plain function in the cpp file.
			[[nodiscard]] BlockId populate(const BlockVec blockCoord, const BasisInfo& basisInfo, const TestGenerator& generator) const noexcept;
	};
}
