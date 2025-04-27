#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	class WorldBaseHeight;

	template<uint64 Seed, Float HAmp, Float HFeatScale>
	class BiomeDebugBaseHeight : public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Range = Layer::ChunkSpanX;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.

			Float get(BIOME_HEIGHT_ARGS) const noexcept {
				return h0 + HAmp * simplex1.value(blockCoordX * HFeatScale, 0); // TODO: 1d simplex
			}

		private:
			// TODO: Shared data/noise at the Generator level.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};
	};

	struct BiomeDebugOneHeight : public BiomeDebugBaseHeight<0xF7F7'F7F7'F7F7'1111, 15.0_f, 0.01_f> {
	};
	
	struct BiomeDebugTwoHeight : public BiomeDebugBaseHeight<0xF7F7'F7F7'F7F7'2222, 30.0_f, 0.02_f> {
	};
	
	struct BiomeDebugThreeHeight : public BiomeDebugBaseHeight<0xF7F7'F7F7'F7F7'3333, 60.0_f, 0.04_f> {
	};

	struct BiomeDebugMountainHeight : public BiomeDebugBaseHeight<0xF7F7'F7F7'F7F7'4444, 60.0_f, 0.04_f> {
		public:
			Float get(BIOME_HEIGHT_ARGS) const noexcept {
				// TODO: To avoid the odd bulges in neighboring biomes we should do something like:
				//       `if (rawInfo.id != this.id) { return h0; }`
				const auto half = rawInfo.size / 2;
				const auto off = half - std::abs(rawInfo.biomeRem.x - half);
				const auto hMargin = 30;
				return h0 + off - hMargin;
			}
	};

	class BiomeDebugOceanHeight : public BiomeDebugBaseHeight<0xF7F7'F7F7'F7F7'5555, 60.0_f, 0.04_f> {
		public:
			Float get(BIOME_HEIGHT_ARGS) const noexcept {
				return h0;
			}
	};

	class BiomeDebugOne {
		public:
			using Height = BiomeDebugOneHeight;
	};

	class BiomeDebugTwo {
		public:
			using Height = BiomeDebugTwoHeight;
	};

	class BiomeDebugThree {
		public:
			using Height = BiomeDebugThreeHeight;
	};

	class BiomeDebugMountain {
		public:
			using Height = BiomeDebugMountainHeight;
	};

	class BiomeDebugOcean {
		public:
			using Height = BiomeDebugOceanHeight;
	};
}
