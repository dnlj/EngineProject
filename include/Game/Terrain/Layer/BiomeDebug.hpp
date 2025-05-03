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

	struct BiomeDebugMountainHeight {
		public:
			using Range = Layer::ChunkSpanX;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.

			Float get(BIOME_HEIGHT_ARGS) const noexcept {
				// TODO: To avoid the odd bulges in neighboring biomes we should do something like:
				//       `if (rawInfo.id != this.id) { return h0; }`
				const auto half = rawInfo.size / 2;
				const auto off = half - std::abs(rawInfo.biomeRem.x - half);
				const auto hMargin = 30;
				return h0 + off - hMargin;
			}
	};

	class BiomeDebugOceanHeight {
		public:
			using Range = Layer::ChunkSpanX;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.

			Float get(BIOME_HEIGHT_ARGS) const noexcept {
				return h0;
			}
	};

	/////////////////////////////////////////////////////////////////////
	
	template<uint64 Seed>
	class BiomeDebugBasisStrength: public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.

			Float get(BIOME_BASIS_STRENGTH_ARGS) const noexcept {
				// These need to be tuned based on biome scales blend dist or else you can get odd clipping type issues.
				return 0.2_f * simplex1.value(FVec2{blockCoord} * 0.003_f)
					 + 0.2_f * simplex2.value(FVec2{blockCoord} * 0.010_f)
					 + 0.1_f * simplex3.value(FVec2{blockCoord} * 0.100_f)
					 + 0.5_f;
			}

		private:
			// TODO: Shared data/noise at the Generator level.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};
	};

	class BiomeDebugOceanBasisStrength: public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.

			constexpr Float get(BIOME_BASIS_STRENGTH_ARGS) const noexcept {
				return 1.0_f;
			}
	};

	/////////////////////////////////////////////////////////////////////
	
	class BiomeDebugOne {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'1111;
			using Height = BiomeDebugBaseHeight<seed, 15.0_f, 0.01_f>;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
	};

	class BiomeDebugTwo {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'2222;
			using Height = BiomeDebugBaseHeight<seed, 30.0_f, 0.02_f>;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
	};

	class BiomeDebugThree {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'3333;
			using Height = BiomeDebugBaseHeight<seed, 60.0_f, 0.04_f>;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
	};

	class BiomeDebugMountain {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'4444;
			using Height = BiomeDebugMountainHeight;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
	};

	class BiomeDebugOcean {
		public:
			using Height = BiomeDebugOceanHeight;
			using BasisStrength = BiomeDebugOceanBasisStrength;
	};
}
