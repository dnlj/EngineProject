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
			entityComponents.resize(eid + 1);
			aliveEntities.resize(eid + 1);
		}

		aliveEntities[eid] = true;
		
		// TODO: detail::onEntityCreatedAll(eid);

		return eid;
	}

	void EntityManager::destroyEntity(EntityID eid) {
		#if defined(DEBUG) 
			if (!isAlive(eid)) {
				ENGINE_ERROR("Attempting to deleting already dead entity with id \"" << eid << "\"");
			}
		#endif

		aliveEntities[eid] = false;
		entityComponents[eid] = 0;
		deadEntities.emplace_back(eid);
		// TODO: detail::onEntityDestroyedAll(eid);
	}

	bool EntityManager::isAlive(EntityID eid) {
		return aliveEntities[eid];
	}
}