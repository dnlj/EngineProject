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
	SystemManager::SystemManager() {
		std::fill(globalToLocalID.begin(), globalToLocalID.end(), static_cast<SystemID>(-1));
	}

	SystemID SystemManager::getNextSystemID() {
		#if defined(DEBUG)
			if (nextID >= MAX_SYSTEMS) {
				ENGINE_ERROR("Attempting to generate an invalid local system id.");
			}
		#endif

		return nextID++;
	}

	SystemID SystemManager::getSystemID(SystemID gsid) {
		const auto sid = globalToLocalID[gsid];

		#if defined(DEBUG)
			if (gsid >= MAX_SYSTEMS) {
				ENGINE_ERROR("Attempting to get the local id of an invalid local system id.");
			}

			if (sid >= nextID) {
				ENGINE_ERROR("Attempting to get the local id of an nonregistered system.");
			}
		#endif

		return sid;
	}
}
