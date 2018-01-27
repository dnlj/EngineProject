// Engine
#include <Engine/ECS/EntityManager.hpp>

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
}