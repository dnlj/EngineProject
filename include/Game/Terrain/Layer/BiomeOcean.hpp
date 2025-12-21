#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/Layer/OnDemandLayer.hpp>
#include <Game/Terrain/StructureInfo.hpp>


namespace Game::Terrain::Layer {
	constexpr inline auto BiomeOceanSeed = 0xF7F7'F7F7'F7F7'5555; // TODO: pull/transform seed from generator.

	class BiomeOceanHeight : public OnDemandLayer {
		public:
			using Partition = ChunkSpanX;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator);
			Float get(BIOME_HEIGHT_ARGS) const noexcept { return h0; }
	};

	class BiomeOceanBasisStrength : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = ChunkArea;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator);
			constexpr static Float get(BIOME_BASIS_STRENGTH_ARGS) noexcept { return 1.0_f; }
	};

	class BiomeOceanBasis : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = ChunkArea;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator);
			Float get(BIOME_BASIS_ARGS) const noexcept;
	};

	class BiomeOceanBlock : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = ChunkArea;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator);
			BlockId get(BIOME_BLOCK_ARGS) const noexcept;
	};

	class BiomeOceanStructureInfo : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = ChunkVec;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator);
			void get(BIOME_STRUCTURE_INFO_ARGS) const noexcept;
	};

	class BiomeOceanStructure : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = ChunkVec;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator);
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
			using BasisStrength = BiomeOceanBasisStrength;
			using Basis = BiomeOceanBasis;
			using Block = BiomeOceanBlock;
			using StructureInfo = BiomeOceanStructureInfo;
			using Structure = BiomeOceanStructure;
			using SharedData = BiomeOceanSharedData;
	};
}
