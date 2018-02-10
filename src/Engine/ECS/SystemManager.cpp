// Engine
#include <Engine/ECS/SystemManager.hpp>

#if defined(DEBUG)
	#include <Engine/Engine.hpp>
#endif

// Static
namespace Engine::ECS {
	SystemID SystemManager::getNextGlobalSystemID() {
		static SystemID id = 0;

		#if defined(DEBUG)
			if (id >= MAX_SYSTEMS) {
				ENGINE_ERROR("Attempting to generate invalid global system id.");
			}
		#endif

		return id++;
	}
}

namespace Engine::ECS {
	SystemID SystemManager::getNextSystemID() {
		#if defined(DEBUG)
			if (nextID >= MAX_SYSTEMS) {
				ENGINE_ERROR("Attempting to generate an invalid local system id.");
			}
		#endif

		return nextID++;
	}

	SystemID SystemManager::getSystemID(SystemID gsid) {
		#if defined(DEBUG)
			if (gsid >= MAX_SYSTEMS) {
				ENGINE_ERROR("Attempting to get the local id of an invalid local system id.");
			}
		#endif

		return globalToLocalID[gsid];
	}
}
