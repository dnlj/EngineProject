#pragma once

// Game
#include <Game/Terrain/Layer/biome.hpp>
#include <Game/Terrain/Layer/BaseBiome.hpp>


namespace Game::Terrain::Layer {
	constexpr inline auto BiomeOceanSeed = 0xF7F7'F7F7'F7F7'5555; // TODO: pull/transform seed from generator.

	class BiomeOceanHeight : public BaseBiomeHeight, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			Float get(BIOME_HEIGHT_ARGS) const noexcept { return h0; }
	};

	class BiomeOceanWeight : public BaseBiomeWeight, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			constexpr static Float get(BIOME_WEIGHT_ARGS) noexcept { return 1.0_f; }
	};

	class BiomeOceanBasis : public BaseBiomeBasis, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			Float get(BIOME_BASIS_ARGS) const noexcept;
	};

	class BiomeOceanBlock : public BaseBiomeBlock, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			BlockId get(BIOME_BLOCK_ARGS) const noexcept;
	};

	class BiomeOceanStructureInfo : public BaseBiomeStructureInfo, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			void get(BIOME_STRUCTURE_INFO_ARGS) const noexcept;
	};

	class BiomeOceanStructure : public BaseBiomeStructure, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			void get(BIOME_STRUCTURE_ARGS) const noexcept;
	};
	
	class BiomeOceanSharedData {
		public:
			// TODO: seed/transform from generator.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(BiomeOceanSeed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(BiomeOceanSeed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(BiomeOceanSeed)))};
	};

	class BiomeOcean {
		public:
			using Height = BiomeOceanHeight;
			using Weight = BiomeOceanWeight;
			using Basis = BiomeOceanBasis;
			using Block = BiomeOceanBlock;
			using StructureInfo = BiomeOceanStructureInfo;
			using Structure = BiomeOceanStructure;
			using SharedData = BiomeOceanSharedData;
	};
}
