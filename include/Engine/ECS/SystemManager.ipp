#pragma once

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
}