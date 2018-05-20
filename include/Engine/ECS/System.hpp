#pragma once

// Engine
#include <Engine/ECS/Common.hpp>

namespace Engine::ECS {
	/**
	 * Abstract class representing a system.
	 */
	class System {
		public:
			/** The bitset of systems to have higher priority than. */
			Engine::ECS::SystemBitset priorityBefore;

			/** The bitset of systems to have lower priority than. */
			Engine::ECS::SystemBitset priorityAfter;

			virtual ~System() = 0;
			virtual void onEntityCreated(EntityID eid) {};
			virtual void onEntityDestroyed(EntityID eid) {};
			virtual void onComponentAdded(EntityID eid, ComponentID cid) {};
			virtual void onComponentRemoved(EntityID eid, ComponentID cid) {};
			virtual void run(float dt) {};
	};
}
