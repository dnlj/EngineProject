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

	template<class System>
	System& SystemManager::getSystem() {
		return *static_cast<System*>(systems[getSystemID<System>()]);
	}

	template<class System1, class System2, class... Systems>
	SystemBitset SystemManager::getBitsetForSystems() {
		return getBitsetForSystems<System1>() |= getBitsetForSystems<System2, Systems...>();
	}

	template<class System>
	SystemBitset SystemManager::getBitsetForSystems() {
		const auto sid = getSystemID<System>();
		SystemBitset value;
		value[sid] = true;
		return value;
	}

	template<class System, class>
	void SystemManager::registerSystem() {
		const auto sid = getNextSystemID();
		globalToLocalID[getGlobalSystemID<System>()] = sid;

		systems[sid] = new System();
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