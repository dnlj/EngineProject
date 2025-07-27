#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/BiomeDebug.hpp>
#include <Game/Terrain/Layer/OnDemandLayer.hpp>

namespace Game::Terrain::Layer {
	constexpr inline auto BiomeMountainSeed = 0xF7F7'F7F7'F7F7'4444; // TODO: pull/transform seed from generator.

	class BiomeMountainHeight : public OnDemandLayer {
		public:
			using Range = ChunkSpanX;
			using Partition = ChunkSpanX;

		public:
			void request(const Range area, TestGenerator& generator);
			Float get(BIOME_HEIGHT_ARGS) const noexcept;
	};

	class BiomeMountainBasis : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			Float get(BIOME_BASIS_ARGS) const noexcept;
	};
	
	class BiomeDebugMountain {
		public:
			constexpr static uint64 seed = BiomeMountainSeed;
			using Height = BiomeMountainHeight;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
			using Basis = BiomeMountainBasis;
			using Block = BiomeDebugBlock<BlockId::Debug4>;
			using SharedData = BiomeDebugSharedData<seed>;
	};
}
