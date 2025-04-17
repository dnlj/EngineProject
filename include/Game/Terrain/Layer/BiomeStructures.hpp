#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	class BiomeStructures : public DependsOn<> {
		public:
			using Range = ChunkArea;
			using Index = Range;

		public:
			void request(const Range chunkArea, TestGenerator& generator);
			void generate(const Range chunkArea, TestGenerator& generator);
			void get(const Index chunkArea, TestGenerator& generator, const RealmId realmId, Terrain& terrain) const noexcept;
	};
}
