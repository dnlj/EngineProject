// Engine
#include <Engine/ECS/EntityManager.hpp>

#if defined(DEBUG)
	#include <Engine/Engine.hpp>
#endif

namespace Engine::ECS {
	EntityID EntityManager::createEntity(bool forceNew) {
		auto eid = aliveEntities.size();

		if (!forceNew && !deadEntities.empty()) {
			eid = deadEntities.back();
			deadEntities.pop_back();
		} else {
			aliveEntities.resize(eid + 1);
		}

		aliveEntities[eid] = true;
		return eid;
	}

	void EntityManager::destroyEntity(EntityID eid) {
		#if defined(DEBUG) 
			if (!isAlive(eid)) {
				ENGINE_ERROR("Attempting to deleting already dead entity with id \"" << eid << "\"");
			}
		#endif

		aliveEntities[eid] = false;
		deadEntities.emplace_back(eid);
	}

	bool EntityManager::isAlive(EntityID eid) {
		return aliveEntities[eid];
	}
}
