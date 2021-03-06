#pragma once

// STD
#include <set>

// Engine
#include <Engine/ECS/Entity.hpp>
#include <Engine/SparseSet.hpp>


namespace Game {
	class ECSNetworkingComponent {
		public:
			enum class NeighborState {
				None,
				Added,
				Current,
				Removed,
			};

			struct NeighborData {
				NeighborState state;
				Engine::ECS::ComponentBitset comps;
			};

			Engine::SparseSet<Engine::ECS::Entity, NeighborData> neighbors;
	};
}
