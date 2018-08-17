#pragma once

namespace Engine::ECS {
	template<class SystemsSet, class ComponentsSet>
	World<SystemsSet, ComponentsSet>::World()
		: fm{em}
		, sm{*this} {
	}

	template<class SystemsSet, class ComponentsSet>
	bool World<SystemsSet, ComponentsSet>::isAlive(Entity ent) const {
		return em.isAlive(ent);
	}

	template<class SystemsSet, class ComponentsSet>
	void World<SystemsSet, ComponentsSet>::setEnabled(Entity ent, bool enabled) {
		em.setEnabled(ent, enabled);
	}

	template<class SystemsSet, class ComponentsSet>
	bool World<SystemsSet, ComponentsSet>::isEnabled(Entity ent) const {
		return em.isEnabled(ent);
	}

	template<class SystemsSet, class ComponentsSet>
	const EntityManager::EntityContainer& World<SystemsSet, ComponentsSet>::getEntities() const {
		return em.getEntities();
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... ComponentN>
	ComponentBitset World<SystemsSet, ComponentsSet>::getBitsetForComponents() const {
		return cm.getBitsetForComponents<ComponentN...>();
	}
	
	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	constexpr static ComponentID World<SystemsSet, ComponentsSet>::getComponentID() noexcept {
		return ComponentManager::getComponentID<Component>();
	}

	template<class SystemsSet, class ComponentsSet>
	template<class System>
	constexpr static SystemID World<SystemsSet, ComponentsSet>::getSystemID() noexcept {
		return SystemManager::getSystemID<System>();
	}

	template<class SystemsSet, class ComponentsSet>
	template<class System>
	System& World<SystemsSet, ComponentsSet>::getSystem() {
		return sm.getSystem<System>();
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... SystemN>
	SystemBitset World<SystemsSet, ComponentsSet>::getBitsetForSystems() const {
		return sm.getBitsetForSystems<SystemN...>();
	};

	template<class SystemsSet, class ComponentsSet>
	void World<SystemsSet, ComponentsSet>::run(float dt) {
		sm.run(dt);
	}

	template<class SystemsSet, class ComponentsSet>
	Entity World<SystemsSet, ComponentsSet>::createEntity(bool forceNew) {
		const auto ent = em.createEntity(forceNew);

		if (ent.id >= cm.componentBitsets.size()) {
			cm.componentBitsets.resize(ent.id + 1);
		} else {
			cm.componentBitsets[ent.id].reset();
		}

		return ent;
	}

	template<class SystemsSet, class ComponentsSet>
	void World<SystemsSet, ComponentsSet>::destroyEntity(Entity ent) {
		em.destroyEntity(ent);
		fm.onEntityDestroyed(ent, cm.componentBitsets[ent.id]);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<SystemsSet, ComponentsSet>::addComponent(Entity ent) {
		auto& container = cm.getComponentContainer<Component>();
		const auto cid = getComponentID<Component>();

		// Ensure the container is of the correct size
		if (ent.id >= container.size()) {
			container.resize(ent.id + 1);
		}

		// Add the component
		cm.componentBitsets[ent.id][cid] = true;

		// Update filters
		fm.onComponentAdded(ent, cid, cm.componentBitsets[ent.id]);

		return container[ent.id];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	std::tuple<Components&...> World<SystemsSet, ComponentsSet>::addComponents(Entity ent) {
		return std::forward_as_tuple(addComponent<Components>(ent) ...);
	}

	template<class SystemsSet, class ComponentsSet>
	bool World<SystemsSet, ComponentsSet>::hasComponent(Entity ent, ComponentID cid) {
		return cm.componentBitsets[ent.id][cid];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	bool World<SystemsSet, ComponentsSet>::hasComponent(Entity ent) {
		return hasComponent(ent, getComponentID<Component>());
	}

	template<class SystemsSet, class ComponentsSet>
	bool World<SystemsSet, ComponentsSet>::hasComponents(Entity ent, ComponentBitset cbits) {
		return (cm.componentBitsets[ent.id] & cbits) == cbits;
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	bool World<SystemsSet, ComponentsSet>::hasComponents(Entity ent) {
		return hasComponents(ent, getBitsetForComponents<Components...>());
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	void World<SystemsSet, ComponentsSet>::removeComponent(Entity ent) {
		removeComponents<Component>(ent);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	void World<SystemsSet, ComponentsSet>::removeComponents(Entity ent) {
		cm.componentBitsets[ent.id] &= ~getBitsetForComponents<Components...>();

		((getComponent<Components>(ent) = Components()), ...);

		// TODO: Make filter manager take bitset?
		(fm.onComponentRemoved(ent, getComponentID<Components>()), ...);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<SystemsSet, ComponentsSet>::getComponent(Entity ent) {
		return cm.getComponentContainer<Component>()[ent.id];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	std::tuple<Components&...> World<SystemsSet, ComponentsSet>::getComponents(Entity ent) {
		return std::forward_as_tuple(getComponent<Components>(ent) ...);
	}

	template<class SystemsSet, class ComponentsSet>
	ComponentBitset World<SystemsSet, ComponentsSet>::getComponentsBitset(Entity ent) const {
		return cm.componentBitsets[ent.id];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	EntityFilter& World<SystemsSet, ComponentsSet>::getFilterFor() {
		return fm.getFilterFor(*this, getBitsetForComponents<Components...>());
	}
}
