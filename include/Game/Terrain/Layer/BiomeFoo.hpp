#pragma once

// Game
#include <Game/Terrain/Layer/biome.hpp>


namespace Game::Terrain::Layer {
	class WorldBaseHeight;
	class BlendedBiomeHeight;

	class BiomeFooHeight : public OnDemandLayer, public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Partition = BlendedBiomeHeight::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition regionCoordX, TestGenerator& generator);
			Float get(BIOME_HEIGHT_ARGS) const noexcept;
	};

	class BiomeFooWeight : public OnDemandLayer, public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Partition = BlendedBiomeWeights::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			Float get(BIOME_BASIS_STRENGTH_ARGS) const noexcept;
	};

	class BiomeFooBasis : public OnDemandLayer, public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Partition = BlendedBiomeBasis::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			Float get(BIOME_BASIS_ARGS) const noexcept;
	};

	class BiomeFooBlock : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = BlendedBiomeBlock::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			BlockId get(BIOME_BLOCK_ARGS) const noexcept;
	};

	class BiomeFooStructureInfo : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = BlendedBiomeStructureInfo::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			void get(BIOME_STRUCTURE_INFO_ARGS) const noexcept;
	};

	class BiomeFooStructure : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Partition = BlendedBiomeStructures::Partition;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Partition area, TestGenerator& generator) = delete;
			void get(BIOME_STRUCTURE_ARGS) const noexcept;
	};

	class BiomeFooSharedData {
		public:
			// TODO: seed/transform from generator.
			Engine::Noise::OpenSimplexNoise simplex{1234};
	};

	// TODO: Doc what layers biomes can have somewhere. and which are optional.
	class BiomeFoo {
		public:
			using Height = BiomeFooHeight;
			using Weight = BiomeFooWeight;
			using Basis = BiomeFooBasis;
			using Block = BiomeFooBlock;
			using StructureInfo = BiomeFooStructureInfo;
			using Structure = BiomeFooStructure;
			using SharedData = BiomeFooSharedData;
	};
}
