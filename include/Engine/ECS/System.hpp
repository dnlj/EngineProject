#pragma once

// Engine
#include <Engine/ECS/Common.hpp>

namespace Engine::ECS {
	// TODO: Doc
	class System {
		public:
			// TODO: Make this use a typeset instead?
			// TODO: Ideally this would be const/static
			/** The bitset of systems to have higher priority than. */
			Engine::ECS::SystemBitset priorityBefore;

			// TODO: Make this use a typeset instead?
			// TODO: Ideally this would be const/static
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
