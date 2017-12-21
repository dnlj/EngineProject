// Engine
#include <Engine/SystemBase.hpp>

namespace Engine {
	void SystemBase::onEntityCreated(Entity ent) {
		if (!hasEntity(ent) && ent.hasComponents(cbits)) {
			addEntity(ent);
		}
	}

	void SystemBase::onComponentAdded(Entity ent) {
		if (!hasEntity(ent) && ent.hasComponents(cbits)) {
			addEntity(ent);
		}
	}

	void SystemBase::onComponentRemoved(Entity ent) {
		if (hasEntity(ent) && !ent.hasComponents(cbits)) {
			removeEntity(ent);
		}
	}

	void SystemBase::onEntityDestroyed(Entity ent) {
		if (hasEntity(ent)) {
			removeEntity(ent);
		}
	}

	void SystemBase::addEntity(Entity ent) {
		auto pos = std::lower_bound(entities.cbegin(), entities.cend(), ent);
		entities.insert(pos, ent);

		auto eid = ent.getID();
		if (hasEntities.size() <= eid) {
			hasEntities.resize(eid + 1);
		}
		hasEntities[eid] = true;
	}

	void SystemBase::removeEntity(Entity ent) {
		auto found = std::lower_bound(entities.cbegin(), entities.cend(), ent);
		entities.erase(found);
		hasEntities[ent.getID()] = false;
	}

	bool SystemBase::hasEntity(Entity ent) {
		auto eid = ent.getID();

		if (hasEntities.size() <= eid) {
			return false;
		} else {
			return hasEntities[eid];
		}
	}
}
