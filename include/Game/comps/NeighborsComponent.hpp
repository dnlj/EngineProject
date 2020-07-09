#pragma once

// STD
#include <set>

// Engine
#include <Engine/ECS/Entity.hpp>
#include <Engine/SparseSet.hpp>


namespace Game {
	class NeighborsComponent {
		public:
			// TODO: SparseSet? Bitmap?
			using Set = Engine::SparseSet<Engine::ECS::Entity, void>;
			Set addedNeighbors;
			Set currentNeighbors;
			Set removedNeighbors;
	};
}
