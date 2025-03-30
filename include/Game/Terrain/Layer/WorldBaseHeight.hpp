#pragma once

#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.

namespace Game::Terrain::Layer {
	// The large world-scale height variation that persists between all biomes.
	class WorldBaseHeight : public DependsOn<> {
		public:
			class Input {
				public:
					BlockUnit xMin;
					BlockUnit xMax;
			};

		public:
			void request(const Input area) {
				// TODO: Reset isn't quite right here. It could/will be possible to have
				//       multiple requests "active" at once/threaded. This is fine
				//       currently because we happen to only have one request at
				//       once/single thread. We will need to revisit this once we get to
				//       multithreading and request optimization.
				h0Cache.reset(area.xMin, area.xMax);
			}

			void generate(const Input area) noexcept {
				// TODO: use _f for Float. Move from TerrainPreview.
				// TODO: keep in mind that this is +- amplitude, and for each octave we increase the contrib;
				// TODO: tune + octaves, atm this is way to steep.
				for (auto x = area.xMin; x < area.xMax; ++x) {
					h0Cache.get(x) = static_cast<BlockUnit>(500 * simplex1.value(x * 0.00005f, 0));
				}
			}

			BlockUnit get(BlockUnit x) const noexcept {
				return h0Cache.get(x);
			}

		// TODO: private, currently just to ease transition to Layers in terrain preview.
		//       Should fix logic there to ensure within bounds. Shouldn't be any need to know
		//       the min/max of the cache.
		public:
			// TODO: May be threading considerations. Maybe have an option to do
			//       independent layer/per thread/top level request?
			// TODO: Would like a way to ask it to clear cache per top level request/"batch" if possible.
			HeightCache h0Cache;

		private:
			// TODO: Should we have a mechanism for sharing noise generators between multiple systems?
			// TODO: should have see as constructor param.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(21212)};
	};
}
