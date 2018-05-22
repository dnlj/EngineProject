#pragma once

namespace Engine::ECS {
	template<class SystemsSet, class ComponentsSet>
	World<SystemsSet, ComponentsSet>::World()
		: SystemManager{*this} {
	}

	template<class SystemsSet, class ComponentsSet>
	EntityID World<SystemsSet, ComponentsSet>::createEntity(bool forceNew) {
		const auto eid = EntityManager::createEntity(forceNew);

		if (eid >= componentBitsets.size()) {
			componentBitsets.resize(eid + 1);
		}

		onEntityCreated(eid);

		return eid;
	}

	template<class SystemsSet, class ComponentsSet>
	void World<SystemsSet, ComponentsSet>::destroyEntity(EntityID eid) {
		EntityManager::destroyEntity(eid);
		onEntityDestroyed(eid);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<SystemsSet, ComponentsSet>::addComponent(EntityID eid) {
		auto& container = getComponentContainer<Component>();
		const auto cid = getComponentID<Component>();

		// Ensure the container is of the correct size
		if (eid >= container.size()) {
			container.resize(eid + 1);
		}

		// Add the component
		container[eid] = Component();
		componentBitsets[eid][cid] = true;

		// Tell the systems
		onComponentAdded(eid, cid);

		return container[eid];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	std::tuple<Components&...> World<SystemsSet, ComponentsSet>::addComponents(EntityID eid) {
		return std::forward_as_tuple(addComponent<Components>(eid) ...);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	bool World<SystemsSet, ComponentsSet>::hasComponent(EntityID eid) {
		return componentBitsets[eid][getComponentID<Component>()];
	}

	template<class SystemsSet, class ComponentsSet>
	bool World<SystemsSet, ComponentsSet>::hasComponents(EntityID eid, ComponentBitset cbits) {
		return (componentBitsets[eid] & cbits) == cbits;
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	void World<SystemsSet, ComponentsSet>::removeComponent(EntityID eid) {
		const auto cid = getComponentID<Component>();

		componentBitsets[eid][cid] = false;
		onComponentRemoved(eid, cid);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	void World<SystemsSet, ComponentsSet>::removeComponents(EntityID eid) {
		(removeComponent<Components>(eid), ...);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<SystemsSet, ComponentsSet>::getComponent(EntityID eid) {
		return getComponentContainer<Component>()[eid];
	}
}
