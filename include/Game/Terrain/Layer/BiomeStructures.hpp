#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/DependsOn.hpp>


namespace Game::Terrain::Layer {
	class BiomeStructures : public DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkArea;
			using Index = Range;

		public:
			void request(const Range chunkArea, TestGenerator& generator);
			ENGINE_INLINE void partition(std::vector<Range>& requests, std::vector<Partition>& partitions) { partitions = std::move(requests); }
			void generate(const Range chunkArea, TestGenerator& generator);
			void get(const Index chunkArea, TestGenerator& generator, const RealmId realmId, Terrain& terrain) const noexcept;
	};
}
