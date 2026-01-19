#pragma once

// Game
#include <Game/Terrain/Layer/biome.hpp>
#include <Game/Terrain/Layer/BaseBiome.hpp>


namespace Game::Terrain::Layer {
	class WorldBaseHeight;
	class BlendedBiomeHeight;

	class BiomeFooHeight : public BaseBiomeHeight, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range<Partition>& regionCoordXs, TestGenerator& generator);
			Float get(BIOME_HEIGHT_ARGS) const noexcept;
	};

	class BiomeFooWeight : public BaseBiomeWeight, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			Float get(BIOME_WEIGHT_ARGS) const noexcept;
	};

	class BiomeFooBasis : public BaseBiomeBasis, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			Float get(BIOME_BASIS_ARGS) const noexcept;
	};

	class BiomeFooBlock : public BaseBiomeBlock, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
			BlockId get(BIOME_BLOCK_ARGS) const noexcept;
	};

	class BiomeFooStructureInfo : public BaseBiomeStructureInfo, public OnDemandLayer {
		public:
			constexpr static ChunkUnit maxStructureExtent = 1;
			using OnDemandLayer::OnDemandLayer;
			void get(BIOME_STRUCTURE_INFO_ARGS) const noexcept;
	};

	class BiomeFooStructure : public BaseBiomeStructure, public OnDemandLayer {
		public:
			using OnDemandLayer::OnDemandLayer;
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
