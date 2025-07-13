#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/ChunkArea.hpp>
#include <Game/Terrain/Layer/DependsOn.hpp>


namespace Game::Terrain::Layer {
	class BiomeStructureInfo : public DependsOn<> {
		public:
			// Biome structures doesn't necessarily need to use the chunk grid, at the
			// moment its just convenient. Any grid size so long as:
			//   size <= biomeBlendDist <= (biomeScaleSmall / 2)
			// would work because that allows us to limit the number of biomes to at most
			// four when we check what structures to generate.
			//
			// That is because biomeBlendDist is effectively the "radius" we sample when
			// generating biomes, and if that is less than half the minimum biome size it
			// means that there will never be more than four neighbors in range.
			//
			// To think about this consider you are in a grid with: width = height =
			// biomeScaleSmall. From the center of a cell go out: biomeBlendDist =
			// biomeScaleSmall / 2. This means at the center all samples will be within
			// the same biome, and if you move in any direction you will have at most four
			// neighbors in sample range (at corner). If you move left from center the right
			// neighbors are out of range etc.
			using Range = ChunkArea;
			using Index = Range;

		public:
			void request(const Range chunkArea, TestGenerator& generator);
			void generate(const Range chunkArea, TestGenerator& generator);

			// TODO: Consider using a BSP tree, quad tree, BVH, etc. some spatial type for
			//       structure storage. That could help with culling becomes an issue, otherwise
			//       you would need ot do N^2 AABB checks. Basically a broad-phase.
			void get(const TestGenerator& generator, const Index chunkArea, std::vector<StructureInfo>& structures) const noexcept;

		private:
			void populate(const ChunkVec chunkCoord, const TestGenerator& generator, std::vector<StructureInfo>& structures) const noexcept;
	};
}
