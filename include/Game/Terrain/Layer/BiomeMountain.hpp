#pragma once

// Game
#include <Game/Terrain/Layer/biome.hpp>
#include <Game/Terrain/Layer/BiomeDebug.hpp>


namespace Game::Terrain::Layer {
	constexpr inline auto BiomeMountainSeed = 0xF7F7'F7F7'F7F7'4444; // TODO: pull/transform seed from generator.

	class BiomeMountainHeight : public OnDemandLayer {
		public:
			using Partition = BlendedBiomeHeight::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			Float get(BIOME_HEIGHT_ARGS) const noexcept;
	};

	class BiomeMountainBasis : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = BlendedBiomeBasis::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			Float get(BIOME_BASIS_ARGS) const noexcept;
	};
	
	class BiomeDebugMountain {
		public:
			constexpr static uint64 seed = BiomeMountainSeed;
			using Height = BiomeMountainHeight;
			using Weight = BiomeDebugWeight<seed>;
			using Basis = BiomeMountainBasis;
			using Block = BiomeDebugBlock<BlockId::Debug4>;
			using SharedData = BiomeDebugSharedData<seed>;
	};
}
