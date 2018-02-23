#pragma once

// Engine
#include <Engine/Engine.hpp>

// Static members
namespace Engine::ECS {
	template<class System, class>
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
		return *static_cast<System*>(systems.system[getSystemID<System>()]);
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

	template<class System, class... Args, class>
	void SystemManager::registerSystem(Args&&... args) {
		const auto gsid = getGlobalSystemID<System>();

		if (globalToLocalID[gsid] != static_cast<SystemID>(-1)) {
			ENGINE_WARN("Each system may only be registered once per SystemManager.");
			return;
		}

		const auto sid = getNextSystemID();

		// Set translation id
		globalToLocalID[gsid] = sid;

		// Create system
		systems.system[sid] = new System(std::forward<Args>(args)...);

		// Get the system
		const auto& system = getSystem<System>();

		// Register functions
		systems.onEntityCreated[sid] = &SystemManager::onEntityCreatedCall<System>;
		systems.onEntityDestroyed[sid] = &SystemManager::onEntityDestroyedCall<System>;
		systems.onComponentAdded[sid] = &SystemManager::onComponentAddedCall<System>;
		systems.onComponentRemoved[sid] = &SystemManager::onComponentRemovedCall<System>;
		systems.run[sid] = &SystemManager::runCall<System>;

		// Update priorities
		systems.priority[sid] |= system.priorityBefore;

		for (size_t i = 0; i < system.priorityAfter.size(); ++i) {
			if (system.priorityAfter[i]) {
				systems.priority[i][sid] = true;
			}
		}
	}

	template<class System>
	void SystemManager::onEntityCreatedCall(EntityID eid) {
		getSystem<System>().onEntityCreated(Entity{eid});
	}

	template<class System>
	void SystemManager::onEntityDestroyedCall(EntityID eid) {
		getSystem<System>().onEntityDestroyed(Entity{eid});
	}

	template<class System>
	void SystemManager::onComponentAddedCall(EntityID eid, ComponentID cid) {
		getSystem<System>().onComponentAdded(Entity{eid}, cid);
	}

	template<class System>
	void SystemManager::onComponentRemovedCall(EntityID eid, ComponentID cid) {
		getSystem<System>().onComponentRemoved(Entity{eid}, cid);
	}
	template<class System>
	void SystemManager::runCall(float dt) {
		getSystem<System>().run(dt);
	}

}