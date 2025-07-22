#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/BiomeDebug.hpp>

namespace Game::Terrain::Layer {
	constexpr inline auto BiomeMountainSeed = 0xF7F7'F7F7'F7F7'4444; // TODO: pull/transform seed from generator.

	struct BiomeMountainHeight {
		public:
			using Range = ChunkSpanX;
			using Partition = ChunkSpanX;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void partition(std::vector<Range>& requests, std::vector<Partition>& partitions) { partitions = std::move(requests); }
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_HEIGHT_ARGS) const noexcept;
	};

	class BiomeMountainBasis : public Layer::DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void partition(std::vector<Range>& requests, std::vector<Partition>& partitions) { partitions = std::move(requests); }
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
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
