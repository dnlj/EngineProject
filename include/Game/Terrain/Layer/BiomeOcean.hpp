#pragma once

// Game
#include <Game/Terrain/Layer/biome.hpp>


namespace Game::Terrain::Layer {
	constexpr inline auto BiomeOceanSeed = 0xF7F7'F7F7'F7F7'5555; // TODO: pull/transform seed from generator.

	class BiomeOceanHeight : public OnDemandLayer {
		public:
			using Partition = BlendedBiomeHeight::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			Float get(BIOME_HEIGHT_ARGS) const noexcept { return h0; }
	};

	class BiomeOceanWeight : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = BlendedBiomeWeights::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			constexpr static Float get(BIOME_WEIGHT_ARGS) noexcept { return 1.0_f; }
	};

	class BiomeOceanBasis : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = BlendedBiomeBasis::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			Float get(BIOME_BASIS_ARGS) const noexcept;
	};

	class BiomeOceanBlock : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = BlendedBiomeBlock::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			BlockId get(BIOME_BLOCK_ARGS) const noexcept;
	};

	class BiomeOceanStructureInfo : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = BlendedBiomeStructureInfo::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			void get(BIOME_STRUCTURE_INFO_ARGS) const noexcept;
	};

	class BiomeOceanStructure : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = BlendedBiomeStructures::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
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
