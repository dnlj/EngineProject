#pragma once

#if defined(DEBUG)
	#include <Engine/Engine.hpp>
#endif

// Static members
namespace Engine::ECS {
	template<class System>
	SystemID SystemManager::getGlobalSystemID() {
		const static SystemID id = getNextGlobalSystemID();
		return id;
	}
}

namespace Engine::ECS {
	template<class System>
	SystemID SystemManager::getSystemID() {
		return getSystemID(getGlobalSystemID<System>());
	}

	template<class System>
	System& SystemManager::getSystem() {
		return *static_cast<System*>(systems[getSystemID<System>()]);
	}

	template<class System, class>
	void SystemManager::registerSystem() {
		#if defined(DEBUG)
			if (systemCount >= MAX_SYSTEMS) {
				ENGINE_ERROR("Attempting to register to many systems. Consider increasing Engine::ECS::MAX_SYSTEMS");
			}
		#endif

		systems[systemCount] = new System();
		++systemCount;
		const auto& system = getSystem<System>();

		// Update priorities
		priority[sid] |= system.priorityBefore;

		for (size_t i = 0; i < system.priorityAfter.size(); ++i) {
			if (system.priorityAfter[i]) {
				priority[i][sid] = true;
			}
		}
	}
}