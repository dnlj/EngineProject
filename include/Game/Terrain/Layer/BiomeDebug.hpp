#pragma once

// Game
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/Layer/OnDemandLayer.hpp>


namespace Game::Terrain::Layer {
	class WorldBaseHeight;
	class BlendedBiomeHeight;

	template<uint64 Seed, Float HAmp, Float HFeatScale>
	class BiomeDebugBaseHeight : public OnDemandLayer, public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Range = ChunkSpanX;
			using Partition = ChunkSpanX;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range area, TestGenerator& generator);
			Float get(BIOME_HEIGHT_ARGS) const noexcept;
	};

	template<uint64 Seed>
	class BiomeDebugBasisStrength : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkArea;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range area, TestGenerator& generator);
			Float get(BIOME_BASIS_STRENGTH_ARGS) const noexcept;
	};

	template<uint64 Seed, Float HAmp, Float HFeatScale, Float BScale, Float BOff, auto BTrans = [](auto b){ return b; }>
	class BiomeDebugBasis : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkArea;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range area, TestGenerator& generator);
			Float get(BIOME_BASIS_ARGS) const noexcept;
	};

	template<BlockId Block, int = 0 /* used to avoid duplicate type in tuple*/>
	class BiomeDebugBlock : public OnDemandLayer, public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkArea;

		public:
			using OnDemandLayer::OnDemandLayer;
			void request(const Range area, TestGenerator& generator);
			constexpr static BlockId get(BIOME_BLOCK_ARGS) noexcept { return Block; };
	};
	
	template<uint64 Seed>
	class BiomeDebugSharedData {
		public:
			// TODO: seed/transform from generator.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
			Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
			Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};
	};

	class BiomeDebugOne {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'1111;
			constexpr static Float HAmp = 15.0_f;
			constexpr static Float HFeatScale = 0.01_f;
			using Height = BiomeDebugBaseHeight<seed, HAmp, HFeatScale>;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
			using Basis = BiomeDebugBasis<seed, HAmp, HFeatScale, 0.03_f, 0.15_f, &std::fabsf>;
			using Block = BiomeDebugBlock<BlockId::Debug>;
			using SharedData = BiomeDebugSharedData<seed>;
	};

	class BiomeDebugTwo {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'2222;
			constexpr static Float HAmp = 30.0_f;
			constexpr static Float HFeatScale = 0.02_f;
			using Height = BiomeDebugBaseHeight<seed, HAmp, HFeatScale>;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
			using Basis = BiomeDebugBasis<seed, HAmp, HFeatScale, 0.06_f, 0.75_f, &std::fabsf>;
			using Block = BiomeDebugBlock<BlockId::Debug2>;
			using SharedData = BiomeDebugSharedData<seed>;
	};

	class BiomeDebugThree {
		public:
			constexpr static uint64 seed = 0xF7F7'F7F7'F7F7'3333;
			constexpr static Float HAmp = 60.0_f;
			constexpr static Float HFeatScale = 0.04_f;
			using Height = BiomeDebugBaseHeight<seed, HAmp, HFeatScale>;
			using BasisStrength = BiomeDebugBasisStrength<seed>;
			using Basis = BiomeDebugBasis<seed, HAmp, HFeatScale, 0.12_f, 0.0_f>;
			using Block = BiomeDebugBlock<BlockId::Debug3>;
			using SharedData = BiomeDebugSharedData<seed>;
	};
}
