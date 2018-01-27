// Engine
#include <Engine/ECS/ComponentManager.hpp>

#if defined(DEBUG)
	#include <Engine/Engine.hpp>
#endif

// Static members
namespace Engine::ECS {
	ComponentID ComponentManager::getNextGlobalComponentID() {
		static ComponentID nextID = 0;

		#if defined(DEBUG)
			if (nextID >= MAX_COMPONENTS) {
				ENGINE_ERROR("Attempting to generate invalid global component ids.");
			}
		#endif

		return nextID++;
	}
}

// Non-static members
namespace Engine::ECS{
	ComponentManager::ComponentManager() {
		nameToID.max_load_factor(0.5f);
	}

	ComponentID ComponentManager::getNextComponentID() {
		#if defined(DEBUG)
			if (nextID >= MAX_COMPONENTS) {
				ENGINE_ERROR("Attempting to generate invalid local component ids.");
			}
		#endif

		return nextID++;
	}

	ComponentID ComponentManager::getComponentID(ComponentID gcid) {
		#if defined(DEBUG)
			if (gcid >= MAX_COMPONENTS) {
				ENGINE_ERROR("Attempting to get the local id of an invalid global component id.");
			}
		#endif

		return globalToLocalID[gcid];
	}
}