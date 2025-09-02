#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/Layer/OnDemandLayer.hpp>


namespace Game::Terrain::Layer {
	class BlendedBiomeStructures : public OnDemandLayer, public DependsOn<> {
		public:
			using Range = ChunkArea;
			using Partition = ChunkVec;
			using Index = Range;

		public:
			using OnDemandLayer::OnDemandLayer;

			void request(const Range chunkArea, TestGenerator& generator);
			void get(const Index chunkArea, TestGenerator& generator, const RealmId realmId, Terrain& terrain) const noexcept;
	};
}
