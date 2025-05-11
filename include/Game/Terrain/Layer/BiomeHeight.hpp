#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.



// h0 = broad, world-scale terrain height variations.
// h1 = biome specific height variations. h1 includes h0 as an input. h1 is
//      currently only used as part of an intermediate step and not stored
//      anywhere.
// h2 = final blended height between all influencing biomes.

namespace Game::Terrain::Layer {
	class BiomeWeights;
	class WorldBaseHeight;

	// The absolute weight of each biome. These are non-normalized.
	class BiomeHeight : public DependsOn<BiomeWeights, WorldBaseHeight> {
		public:
			using Range = RegionSpanX;
			using Index = BlockUnit;

		public: // TODO: private, currently public during transition to layers.
			BlockSpanCache<BlockUnit> cache;

		public:
			void request(const Range area, TestGenerator& generator);
			void generate(const Range area, TestGenerator& generator);
			ENGINE_INLINE_REL [[nodiscard]] BlockUnit get(const Index x) const noexcept { return cache.at(x); }

		private:
			[[nodiscard]] BiomeBlend populate(BlockVec blockCoord, const TestGenerator& generator) const noexcept;
	};
}

