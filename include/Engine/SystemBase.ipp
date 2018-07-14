// Engine
#include <Engine/SystemBase.hpp>


namespace Engine {
	template<class SystemsSet, class ComponentsSet>
	SystemBase<ECS::World<SystemsSet, ComponentsSet>>::SystemBase(World& world) : world{world} {
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onEntityCreated(ECS::Entity ent) {
		if (!hasEntity(ent) && world.hasComponents(ent, cbits)) {
			addEntity(ent);
		}
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onComponentAdded(ECS::Entity ent, ECS::ComponentID cid) {
		if (!hasEntity(ent) && world.hasComponents(ent, cbits)) {
			addEntity(ent);
		}
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onComponentRemoved(ECS::Entity ent, ECS::ComponentID cid) {
		if (hasEntity(ent) && cbits[cid]) {
			removeEntity(ent);
		}
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onEntityDestroyed(ECS::Entity ent) {
		if (hasEntity(ent)) {
			removeEntity(ent);
		}
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::addEntity(ECS::Entity ent) {
		auto pos = std::lower_bound(entities.cbegin(), entities.cend(), ent);
		entities.insert(pos, ent);

		if (hasEntities.size() <= ent.id) {
			hasEntities.resize(ent.id + 1);
		}

		hasEntities[ent.id] = true;

		onEntityAdded(ent);
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::removeEntity(ECS::Entity ent) {
		auto found = std::lower_bound(entities.cbegin(), entities.cend(), ent);
		entities.erase(found);
		hasEntities[ent.id] = false;

		onEntityRemoved(ent);
	}

	template<class SystemsSet, class ComponentsSet>
	bool SystemBase<ECS::World<SystemsSet, ComponentsSet>>::hasEntity(ECS::Entity ent) {
		if (hasEntities.size() <= ent.id) {
			return false;
		} else {
			return hasEntities[ent.id];
		}
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onEntityAdded(ECS::Entity ent) {
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onEntityRemoved(ECS::Entity ent) {
	}
}
