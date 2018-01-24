// Engine
#include <Engine/ECS/EntityManager.hpp>

namespace Engine::ECS {
	EntityID EntityManager::createEntity(bool forceNew) {
		auto eid = entityComponents.size();

		if (!forceNew && deadEntities.empty()) {
			eid = deadEntities.back();
			deadEntities.pop_back();
		} else {
			entityComponents.resize(eid + 1);
			entityAlive.resize(eid + 1);
		}

		entityAlive[eid] = true;

		// TODO: onEntityCreatedAll(eid);
		return eid;
	}

	void EntityManager::destroyEntity(EntityID eid) {
		deadEntities.emplace_back(eid);
		entityAlive[eid] = false;
		entityComponents[eid] = 0;
		// TODO: onEntityDestroyedAll
	}


}