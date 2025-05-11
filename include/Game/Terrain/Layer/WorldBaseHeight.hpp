#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.

namespace Game::Terrain::Layer {
	// The large world-scale height variation that persists between all biomes.
	class WorldBaseHeight : public DependsOn<> {
		public:
			using Range = RegionSpanX;
			using Index = BlockUnit;

		public:
			// TODO: May be threading considerations. Maybe have an option to do
			//       independent layer/per thread/top level request?
			// TODO: Would like a way to ask it to clear cache per top level request/"batch" if possible.
			BlockSpanCache<BlockUnit> cache;

		public:
			void request(const Range area, TestGenerator& generator) {
				ENGINE_LOG2("WorldBaseHeight::request {}", area);
				cache.reserve(area);
			}

			void generate(const Range area, TestGenerator& generator) noexcept {
				// TODO: use _f for Float. Move from TerrainPreview.
				// TODO: keep in mind that this is +- amplitude, and for each octave we increase the contrib;
				// TODO: tune + octaves, atm this is way to steep.
				auto cur = cache.walk(area);
				while (cur) {
					const auto blockCoordX = cur.getBlockCoord();
					*cur = static_cast<BlockUnit>(500 * simplex1.value(blockCoordX * 0.00005f, 0));
					++cur;
				}
			}

			// TODO: remove, temp during biome span region transition.
			ENGINE_INLINE_REL [[nodiscard]] BlockUnit get(const Index x) const noexcept {
				return cache.at(x);
			}
			 
			//ENGINE_INLINE_REL [[nodiscard]] BlockUnit get(const Range area) const noexcept {
			//	return cache.walk(area);
			//}

		private:
			// TODO: Should we have a mechanism for sharing noise generators between multiple systems?
			// TODO: should have see as constructor param.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(21212)};
	};
}
