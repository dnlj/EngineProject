// Engine
#include <Engine/SystemBase.hpp>


namespace Engine {
	template<class SystemsSet, class ComponentsSet>
	SystemBase<ECS::World<SystemsSet, ComponentsSet>>::SystemBase(World& world) : world{world} {
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onEntityCreated(ECS::EntityID eid) {
		//if (!hasEntity(eid) && world.hasComponents(eid, cbits)) {
		//	addEntity(eid);
		//}
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onComponentAdded(ECS::EntityID eid, ECS::ComponentID cid) {
		//if (!hasEntity(eid) && world.hasComponents(eid, cbits)) {
		//	addEntity(eid);
		//}
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onComponentRemoved(ECS::EntityID eid, ECS::ComponentID cid) {
		if (hasEntity(eid) && cbits[cid]) {
			removeEntity(eid);
		}
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onEntityDestroyed(ECS::EntityID eid) {
		if (hasEntity(eid)) {
			removeEntity(eid);
		}
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::addEntity(ECS::EntityID eid) {
		auto pos = std::lower_bound(entities.cbegin(), entities.cend(), eid);
		entities.insert(pos, eid);

		if (hasEntities.size() <= eid) {
			hasEntities.resize(eid + 1);
		}
		hasEntities[eid] = true;
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::removeEntity(ECS::EntityID eid) {
		auto found = std::lower_bound(entities.cbegin(), entities.cend(), eid);
		entities.erase(found);
		hasEntities[eid] = false;
	}

	template<class SystemsSet, class ComponentsSet>
	bool SystemBase<ECS::World<SystemsSet, ComponentsSet>>::hasEntity(ECS::EntityID eid) {
		if (hasEntities.size() <= eid) {
			return false;
		} else {
			return hasEntities[eid];
		}
	}
}
