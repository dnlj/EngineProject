#pragma once

#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.

// TODO: Need calc biome first.
namespace Game::Terrain::Layer {
	class BiomeWeights;
	class WorldBaseHeight;

	// The absolute weight of each biome. These are non-normalized.
	class BiomeHeight : public DependsOn<BiomeWeights, WorldBaseHeight> {
		public:
			class Range {
				public:
					BlockUnit xMin;
					BlockUnit xMax;
			};

			using Index = BlockUnit;

		private:
			HeightCache cache;

		public:
			void request(const Range area, TestGenerator& generator);
			void generate(const Range area, TestGenerator& generator);
			ENGINE_INLINE_REL [[nodiscard]] BlockUnit get(const Index x) const noexcept { return cache.get(x); }

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

