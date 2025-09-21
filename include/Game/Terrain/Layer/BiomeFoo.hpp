#pragma once

// Game
#include <Game/Terrain/terrain.hpp>
#include <Game/Terrain/Layer/BiomeDebug.hpp>
#include <Game/Terrain/Layer/OnDemandLayer.hpp>
#include <Game/Terrain/StructureInfo.hpp>

namespace Game::Terrain::Layer {
	class WorldBaseHeight;
	class BlendedBiomeHeight;

	class BiomeFooHeight : public OnDemandLayer, public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Range = ChunkSpanX;
			using Partition = ChunkSpanX;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range area, TestGenerator& generator);
			Float get(BIOME_HEIGHT_ARGS) const noexcept;
	};

	class BiomeFooBasisStrength : public OnDemandLayer, public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkVec;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range area, TestGenerator& generator);
			Float get(BIOME_BASIS_STRENGTH_ARGS) const noexcept;
	};

	class BiomeFooBasis : public OnDemandLayer, public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkVec;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range area, TestGenerator& generator);
			Float get(BIOME_BASIS_ARGS) const noexcept;
	};

	class BiomeFooStructureInfo : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkVec;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range area, TestGenerator& generator);
			void get(BIOME_STRUCTURE_INFO_ARGS) const noexcept;
	};

	class BiomeFooStructure : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkVec;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range area, TestGenerator& generator);
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
			using BasisStrength = BiomeFooBasisStrength;
			using Basis = BiomeFooBasis;
			using Block = BiomeDebugBlock<BlockId::Debug, 1>;
			using StructureInfo = BiomeFooStructureInfo;
			using Structure = BiomeFooStructure;
			using SharedData = BiomeFooSharedData;
	};
}
