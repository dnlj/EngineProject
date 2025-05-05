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
}


#include <Game/Terrain/Layer/BiomeDebug.ipp>
