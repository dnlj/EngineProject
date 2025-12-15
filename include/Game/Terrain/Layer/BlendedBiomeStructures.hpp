#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/Layer/OnDemandLayer.hpp>


namespace Game::Terrain::Layer {
	class BlendedBiomeStructures : public OnDemandLayer, public DependsOn<> {
		public:
			using Partition = ChunkVec;
			using Index = Partition;

		public:
			using OnDemandLayer::OnDemandLayer;

			void request(const Partition chunkCoord, TestGenerator& generator);
			void get(const Index chunkCoord, TestGenerator& generator, const RealmId realmId, Terrain& terrain) const noexcept;
	};
}
