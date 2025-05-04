#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	class WorldBaseHeight;
	class BiomeHeight;

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
			Float get(BIOME_HEIGHT_ARGS) const noexcept { return h0; }
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
			constexpr static Float get(BIOME_BASIS_STRENGTH_ARGS) noexcept { return 1.0_f; }
	};

	/////////////////////////////////////////////////////////////////////
	
	template<uint64 Seed, Float HAmp, Float HFeatScale, Float BScale, Float BOff, auto BTrans = [](auto b){ return b; }>
	class BiomeDebugBasis : public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_BASIS_ARGS) const noexcept;

		private:
			// TODO: Shared data/noise at the Generator level.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};
	};

	template<uint64 Seed>
	class BiomeDebugMountainBasis : public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_BASIS_ARGS) const noexcept;

		private:
			// TODO: Shared data/noise at the Generator level.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};
	};

	template<uint64 Seed>
	class BiomeDebugOceanBasis : public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_BASIS_ARGS) const noexcept;

		private:
			// TODO: Shared data/noise at the Generator level.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};
	};

	/////////////////////////////////////////////////////////////////////

	template<BlockId Block, int = 0 /* used to avoid duplicate type in tuple*/>
	class BiomeDebugBlock : public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			constexpr static BlockId get(BIOME_BLOCK_ARGS) noexcept { return Block; };
	};

	template<uint64 Seed>
	class BiomeDebugOceanBlock : public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			constexpr BlockId get(BIOME_BLOCK_ARGS) const noexcept {
				auto thresh = 0.45_f;

				// TODO: Shouldn't these use simplex 1/2/3 instead of 1/1/2?
				thresh += 0.04_f * simplex1.value(FVec2{blockCoord} * 0.025_f);
				thresh += 0.02_f * simplex1.value(FVec2{blockCoord} * 0.05_f);
				thresh += 0.01_f + 0.01_f * simplex2.value(FVec2{blockCoord} * 0.1_f);

				if (basisInfo.weight > thresh) {
					return BlockId::Grass;
				}

				return BlockId::Gold;
			};

		private:
			// TODO: Shared data/noise at the Generator level.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};
	};

	/////////////////////////////////////////////////////////////////////
	
	class BiomeDebugOne {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'1111;
			constexpr static Float HAmp = 15.0_f;
			constexpr static Float HFeatScale = 0.01_f;
			using Height = BiomeDebugBaseHeight<seed, HAmp, HFeatScale>;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
			using Basis = BiomeDebugBasis<seed, HAmp, HFeatScale, 0.03_f, 0.15_f, &std::fabsf>;
			using Block = BiomeDebugBlock<BlockId::Debug>;
	};

	class BiomeDebugTwo {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'2222;
			constexpr static Float HAmp = 30.0_f;
			constexpr static Float HFeatScale = 0.02_f;
			using Height = BiomeDebugBaseHeight<seed, HAmp, HFeatScale>;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
			using Basis = BiomeDebugBasis<seed, HAmp, HFeatScale, 0.06_f, 0.75_f, &std::fabsf>;
			using Block = BiomeDebugBlock<BlockId::Debug2>;
	};

	class BiomeDebugThree {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'3333;
			constexpr static Float HAmp = 60.0_f;
			constexpr static Float HFeatScale = 0.04_f;
			using Height = BiomeDebugBaseHeight<seed, HAmp, HFeatScale>;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
			using Basis = BiomeDebugBasis<seed, HAmp, HFeatScale, 0.12_f, 0.0_f>;
			using Block = BiomeDebugBlock<BlockId::Debug3>;
	};

	class BiomeDebugMountain {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'4444;
			using Height = BiomeDebugMountainHeight;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
			using Basis = BiomeDebugMountainBasis<seed>;
			using Block = BiomeDebugBlock<BlockId::Debug4>;
	};

	class BiomeDebugOcean {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'5555;
			using Height = BiomeDebugOceanHeight;
			using BasisStrength = BiomeDebugOceanBasisStrength;
			using Basis = BiomeDebugOceanBasis<seed>;
			using Block = BiomeDebugOceanBlock<seed>;
	};
}


#include <Game/Terrain/Layer/BiomeDebug.ipp>
