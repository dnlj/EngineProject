#pragma once

// Game
#include <Game/Terrain/Layer/biome.hpp>

namespace Game::Terrain::Layer {
	class BaseBiomeHeight {
		public:
			using Partition = BlendedBiomeHeight::Partition;
			void request(const Range<Partition>& regionCoordXs, TestGenerator& generator) = delete;
	};

	class BaseBiomeWeight {
		public:
			using Partition = BlendedBiomeWeights::Partition;
			void request(const Range<Partition>& chunkCoords, TestGenerator& generator) = delete;
	};

	class BaseBiomeBasis {
		public:
			using Partition = BlendedBiomeBasis::Partition;
			void request(const Range<Partition>& chunkCoords, TestGenerator& generator) = delete;
	};

	class BaseBiomeBlock {
		public:
			using Partition = BlendedBiomeBlock::Partition;
			void request(const Range<Partition>& chunkCoords, TestGenerator& generator) = delete;
	};

	class BaseBiomeStructureInfo {
		public:
			// Structures should also define the maxStructExtent. Assuming your structures are
			// randomly distributed within chunk, this is effectively the maximum structure size
			// rounded up to the nearest chunk.
			//constexpr ChunkUnit maxStructExtent = 0;

			using Partition = BlendedBiomeStructureInfo::Partition;
			void request(const Range<Partition>& chunkCoords, TestGenerator& generator) = delete;
	};

	class BaseBiomeStructure {
		public:
			using Partition = BlendedBiomeStructures::Partition;
			void request(const Range<Partition>& chunkCoords, TestGenerator& generator) = delete;
	};
}
