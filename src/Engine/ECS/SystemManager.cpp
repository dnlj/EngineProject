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
			if (id >= MAX_SYSTEMS_GLOBAL) {
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
			if (sid >= nextID) {
				ENGINE_ERROR("Attempting to get the local id of an nonregistered system.");
			}
		#endif

		return sid;
	}

	void SystemManager::onEntityCreated(EntityID eid) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.onEntityCreated[i])(eid);
		}
	}

	void SystemManager::onComponentAdded(EntityID eid, ComponentID cid) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.onComponentAdded[i])(eid, cid);
		}
	}

	void SystemManager::onComponentRemoved(EntityID eid, ComponentID cid) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.onComponentRemoved[i])(eid, cid);
		}
	}

	void SystemManager::onEntityDestroyed(EntityID eid) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.onEntityDestroyed[i])(eid);
		}
	}

	void SystemManager::run(float dt) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.run[i])(dt);
		}
	}
}
