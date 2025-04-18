#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.

namespace Game::Terrain::Layer {
	// The large world-scale height variation that persists between all biomes.
	class WorldBaseHeight : public DependsOn<> {
		public:
			using Range = ChunkSpanX;
			using Index = BlockUnit;

		public:
			// TODO: May be threading considerations. Maybe have an option to do
			//       independent layer/per thread/top level request?
			// TODO: Would like a way to ask it to clear cache per top level request/"batch" if possible.
			BlockSpanCache<BlockUnit> cache;

		public:
			void request(const Range area, TestGenerator& generator) {
				// TODO: Reset isn't quite right here. It could/will be possible to have
				//       multiple requests "active" at once/threaded. This is fine
				//       currently because we happen to only have one request at
				//       once/single thread. We will need to revisit this once we get to
				//       multithreading and request optimization.
				cache.reserve(area);
			}

			void generate(const Range area, TestGenerator& generator) noexcept {
				// TODO: use _f for Float. Move from TerrainPreview.
				// TODO: keep in mind that this is +- amplitude, and for each octave we increase the contrib;
				// TODO: tune + octaves, atm this is way to steep.
				//for (auto x = area.min; x < area.max; ++x) {
				//	cache.get(x) = static_cast<BlockUnit>(500 * simplex1.value(x * 0.00005f, 0));
				//}

				cache.forEachBlock(area, [&](const BlockUnit x, BlockUnit& h0) ENGINE_INLINE_REL {
					h0 = static_cast<BlockUnit>(500 * simplex1.value(x * 0.00005f, 0));
				});
			}

			ENGINE_INLINE_REL [[nodiscard]] BlockUnit get(const Index x) const noexcept {
				return cache.at(x);
			}

		private:
			// TODO: Should we have a mechanism for sharing noise generators between multiple systems?
			// TODO: should have see as constructor param.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(21212)};
	};
}
