#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/DependsOn.hpp>


namespace Game::Terrain::Layer {
	class BlendedBiomeStructures : public DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkVec;
			using Index = Range;

		public:
			void request(const Range chunkArea, TestGenerator& generator);

			ENGINE_INLINE void partition(std::vector<Range>& requests, std::vector<Partition>& partitions) {
				// Do nothing. Generated on-demand. See .cpp file.
			}

			void generate(const Partition chunkCoord, TestGenerator& generator);
			void get(const Index chunkArea, TestGenerator& generator, const RealmId realmId, Terrain& terrain) const noexcept;
	};
}
