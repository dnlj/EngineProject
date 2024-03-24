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
				ZoneChanged,
			};

			struct NeighborData {
				NeighborState state;
				Engine::ECS::ComponentBitset comps;
			};

			// TODO: hashmap would probably be better suited here.
			Engine::SparseSet<Engine::ECS::Entity, NeighborData> neighbors;

			// We need to track when the players zone has changed in addition to
			// the neighbors so that ECS_ZONE_INFO still gets sent even if there
			// are no entities near by.
			bool plyZoneChanged = false;
	};
}
