#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/BiomeDebug.hpp>

namespace Game::Terrain::Layer {
	constexpr inline auto BiomeMountainSeed = 0xF7F7'F7F7'F7F7'4444; // TODO: pull/transform seed from generator.

	struct BiomeMountainHeight {
		public:
			using Range = Layer::ChunkSpanX;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_HEIGHT_ARGS) const noexcept;
	};

	class BiomeMountainBasis : public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_BASIS_ARGS) const noexcept;

		private:
			// TODO: Shared data/noise at the Generator level.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(BiomeMountainSeed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(BiomeMountainSeed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(BiomeMountainSeed)))};
	};

	class BiomeDebugMountain {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'4444;
			using Height = BiomeMountainHeight;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
			using Basis = BiomeMountainBasis;
			using Block = BiomeDebugBlock<BlockId::Debug4>;
	};
}
