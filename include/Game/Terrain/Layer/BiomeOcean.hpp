#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	constexpr inline auto BiomeOceanSeed = 0xF7F7'F7F7'F7F7'5555; // TODO: pull/transform seed from generator.

	class BiomeOceanHeight {
		public:
			using Range = Layer::ChunkSpanX;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_HEIGHT_ARGS) const noexcept { return h0; }
	};

	class BiomeOceanBasisStrength: public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			constexpr static Float get(BIOME_BASIS_STRENGTH_ARGS) noexcept { return 1.0_f; }
	};

	class BiomeOceanBasis : public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_BASIS_ARGS) const noexcept;

		private:
			// TODO: Shared data/noise at the Generator level.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(BiomeOceanSeed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(BiomeOceanSeed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(BiomeOceanSeed)))};
	};

	class BiomeOceanBlock : public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			BlockId get(BIOME_BLOCK_ARGS) const noexcept;

		private:
			// TODO: Shared data/noise at the Generator level.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(BiomeOceanSeed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(BiomeOceanSeed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(BiomeOceanSeed)))};
	};

	class BiomeOcean {
		public:
			using Height = BiomeOceanHeight;
			using BasisStrength = BiomeOceanBasisStrength;
			using Basis = BiomeOceanBasis;
			using Block = BiomeOceanBlock;
	};
}
