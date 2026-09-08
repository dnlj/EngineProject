#pragma once

// Engine 
#include <Engine/Hash.hpp>

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/StructureInfo.hpp>
#include <Game/Terrain/Layer/DependsOn.hpp>
#include <Game/Terrain/Layer/OnDemandLayer.hpp>

namespace Game::Terrain::Layer {

	/**
	 * Pre layer to evaluate biome info an request the needed chunks.
	 */
	class BlendedBiomeStructuresEvaluator : public OnDemandLayer, public DependsOn<> {
		public:
			using Partition = UniversalChunkCoord;
			using Index = Partition;

		public:
			using OnDemandLayer::OnDemandLayer;

			void request(const Range<Partition>& chunkCoords, TestGenerator& generator);
	};

	class BlendedBiomeStructures : public OnDemandLayer, public DependsOn<> {
		public:
			using Partition = UniversalChunkCoord;
			using Index = Partition;

		private:
			ENGINE_DEBUG_ONLY(mutable Engine::FlatHashSet<StructureInfo> generatedStructures);

		public:
			using OnDemandLayer::OnDemandLayer;

			void request(const Range<Partition>& chunkCoords, TestGenerator& generator);
			void get(const Index chunkCoord, TestGenerator& generator, Terrain& terrain) const noexcept;
	};
}
