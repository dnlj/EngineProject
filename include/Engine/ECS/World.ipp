#pragma once

namespace Engine::ECS {
	template<class SystemsSet, class ComponentsSet>
	World<SystemsSet, ComponentsSet>::World()
		: SystemManager{*this} {
	}

	template<class SystemsSet, class ComponentsSet>
	Entity World<SystemsSet, ComponentsSet>::createEntity(bool forceNew) {
		const auto ent = EntityManager::createEntity(forceNew);

		if (ent.id >= componentBitsets.size()) {
			componentBitsets.resize(ent.id + 1);
		}

		return ent;
	}

	template<class SystemsSet, class ComponentsSet>
	void World<SystemsSet, ComponentsSet>::destroyEntity(Entity ent) {
		EntityManager::destroyEntity(ent);
		FilterManager::onEntityDestroyed(ent, componentBitsets[ent.id]);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<SystemsSet, ComponentsSet>::addComponent(Entity ent) {
		auto& container = getComponentContainer<Component>();
		const auto cid = getComponentID<Component>();

		// Ensure the container is of the correct size
		if (ent.id >= container.size()) {
			container.resize(ent.id + 1);
		}

		// Add the component
		container[ent.id] = Component();
		componentBitsets[ent.id][cid] = true;

		// Update filters
		FilterManager::onComponentAdded(ent, cid, componentBitsets[ent.id]);

		return container[ent.id];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	std::tuple<Components&...> World<SystemsSet, ComponentsSet>::addComponents(Entity ent) {
		return std::forward_as_tuple(addComponent<Components>(ent) ...);
	}

	template<class SystemsSet, class ComponentsSet>
	bool World<SystemsSet, ComponentsSet>::hasComponent(Entity ent, ComponentID cid) {
		return componentBitsets[ent.id][cid];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	bool World<SystemsSet, ComponentsSet>::hasComponent(Entity ent) {
		return hasComponent(ent, getComponentID<Component>());
	}

	template<class SystemsSet, class ComponentsSet>
	bool World<SystemsSet, ComponentsSet>::hasComponents(Entity ent, ComponentBitset cbits) {
		return (componentBitsets[ent.id] & cbits) == cbits;
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	bool World<SystemsSet, ComponentsSet>::hasComponents(Entity ent) {
		return hasComponents(ent, getBitsetForComponents<Components...>());
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	void World<SystemsSet, ComponentsSet>::removeComponent(Entity ent) {
		const auto cid = getComponentID<Component>();

		componentBitsets[ent.id][cid] = false;
		FilterManager::onComponentRemoved(ent, cid);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	void World<SystemsSet, ComponentsSet>::removeComponents(Entity ent) {
		(removeComponent<Components>(ent), ...);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<SystemsSet, ComponentsSet>::getComponent(Entity ent) {
		return getComponentContainer<Component>()[ent.id];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	std::tuple<Components&...> World<SystemsSet, ComponentsSet>::getComponents(Entity ent) {
		return std::forward_as_tuple(getComponent<Components>(ent) ...);
	}

	template<class SystemsSet, class ComponentsSet>
	ComponentBitset World<SystemsSet, ComponentsSet>::getComponentsBitset(Entity ent) const {
		return componentBitsets[ent.id];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	EntityFilter& World<SystemsSet, ComponentsSet>::getFilterFor() {
		return FilterManager::getFilterFor(*this, getBitsetForComponents<Components...>());
	}
}
