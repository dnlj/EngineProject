#pragma once

// Engine
#include <Engine/ECS/World.hpp>


namespace Engine::ECS {
	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class Arg>
	World<TickRate, SystemsSet, ComponentsSet>::World(float tickInterval, Arg& arg)
		: fm{em}
		, sm{arg}
		, beginTime{TimeClock::now()} {
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	void World<TickRate, SystemsSet, ComponentsSet>::run() {
		const auto endTime = TimeClock::now();
		deltaTimeNS = endTime - beginTime;
		beginTime = endTime;
		deltaTime = std::chrono::duration_cast<TimeDurationSeconds>(deltaTimeNS).count();

		tickAccum += deltaTimeNS;
		while (tickInterval < tickAccum) {
			constexpr auto tickDelta = std::chrono::duration_cast<TimeDurationSeconds>(tickInterval).count();
			sm.tick(tickDelta);
			tickAccum -= tickInterval;
		}

		sm.run(deltaTime);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	bool World<TickRate, SystemsSet, ComponentsSet>::isAlive(Entity ent) const {
		return em.isAlive(ent);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	void World<TickRate, SystemsSet, ComponentsSet>::setEnabled(Entity ent, bool enabled) {
		em.setEnabled(ent, enabled);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	bool World<TickRate, SystemsSet, ComponentsSet>::isEnabled(Entity ent) const {
		return em.isEnabled(ent);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	const EntityManager::EntityContainer& World<TickRate, SystemsSet, ComponentsSet>::getEntities() const {
		return em.getEntities();
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class... ComponentN>
	ComponentBitset World<TickRate, SystemsSet, ComponentsSet>::getBitsetForComponents() const {
		return cm.getBitsetForComponents<ComponentN...>();
	}
	
	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class Component>
	constexpr static ComponentID World<TickRate, SystemsSet, ComponentsSet>::getComponentID() noexcept {
		return ComponentManager::getComponentID<Component>();
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class System>
	constexpr static SystemID World<TickRate, SystemsSet, ComponentsSet>::getSystemID() noexcept {
		return SystemManager::getSystemID<System>();
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class System>
	System& World<TickRate, SystemsSet, ComponentsSet>::getSystem() {
		return sm.getSystem<System>();
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class... SystemN>
	SystemBitset World<TickRate, SystemsSet, ComponentsSet>::getBitsetForSystems() const {
		return sm.getBitsetForSystems<SystemN...>();
	};

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	void World<TickRate, SystemsSet, ComponentsSet>::run(float dt) {
		sm.run(dt);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	Entity World<TickRate, SystemsSet, ComponentsSet>::createEntity(bool forceNew) {
		const auto ent = em.createEntity(forceNew);

		if (ent.id >= cm.componentBitsets.size()) {
			cm.componentBitsets.resize(ent.id + 1);
		} else {
			cm.componentBitsets[ent.id].reset();
		}

		return ent;
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	void World<TickRate, SystemsSet, ComponentsSet>::destroyEntity(Entity ent) {
		em.destroyEntity(ent);
		fm.onEntityDestroyed(ent, cm.componentBitsets[ent.id]);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<TickRate, SystemsSet, ComponentsSet>::addComponent(Entity ent) {
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

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class... Components>
	std::tuple<Components&...> World<TickRate, SystemsSet, ComponentsSet>::addComponents(Entity ent) {
		return std::forward_as_tuple(addComponent<Components>(ent) ...);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	bool World<TickRate, SystemsSet, ComponentsSet>::hasComponent(Entity ent, ComponentID cid) {
		return cm.componentBitsets[ent.id][cid];
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class Component>
	bool World<TickRate, SystemsSet, ComponentsSet>::hasComponent(Entity ent) {
		return hasComponent(ent, getComponentID<Component>());
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	bool World<TickRate, SystemsSet, ComponentsSet>::hasComponents(Entity ent, ComponentBitset cbits) {
		return (cm.componentBitsets[ent.id] & cbits) == cbits;
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class... Components>
	bool World<TickRate, SystemsSet, ComponentsSet>::hasComponents(Entity ent) {
		return hasComponents(ent, getBitsetForComponents<Components...>());
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class Component>
	void World<TickRate, SystemsSet, ComponentsSet>::removeComponent(Entity ent) {
		removeComponents<Component>(ent);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class... Components>
	void World<TickRate, SystemsSet, ComponentsSet>::removeComponents(Entity ent) {
		cm.componentBitsets[ent.id] &= ~getBitsetForComponents<Components...>();

		((getComponent<Components>(ent) = Components()), ...);

		// TODO: Make filter manager take bitset?
		(fm.onComponentRemoved(ent, getComponentID<Components>()), ...);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<TickRate, SystemsSet, ComponentsSet>::getComponent(Entity ent) {
		return cm.getComponentContainer<Component>()[ent.id];
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class... Components>
	std::tuple<Components&...> World<TickRate, SystemsSet, ComponentsSet>::getComponents(Entity ent) {
		return std::forward_as_tuple(getComponent<Components>(ent) ...);
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	ComponentBitset World<TickRate, SystemsSet, ComponentsSet>::getComponentsBitset(Entity ent) const {
		return cm.componentBitsets[ent.id];
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class... Components>
	EntityFilter& World<TickRate, SystemsSet, ComponentsSet>::getFilterFor() {
		return fm.getFilterFor(*this, getBitsetForComponents<Components...>());
	}
	
	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	auto World<TickRate, SystemsSet, ComponentsSet>::getTickInterval() const {
		return tickInterval;
	}
	
	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	auto World<TickRate, SystemsSet, ComponentsSet>::getTickAccumulation() const {
		return tickAccum;
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	float32 World<TickRate, SystemsSet, ComponentsSet>::getTickRatio() const {
		return tickAccum.count() / static_cast<float>(tickInterval.count());
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	float32 World<TickRate, SystemsSet, ComponentsSet>::getDeltaTime() const {
		return deltaTime;
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	auto World<TickRate, SystemsSet, ComponentsSet>::getDeltaTimeNS() const {
		return deltaTimeNS;
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class SystemA, class SystemB>
	constexpr static bool World<TickRate, SystemsSet, ComponentsSet>::orderBefore() {
		return SystemManager::orderBefore<SystemA, SystemB>();
	}

	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	template<class SystemA, class SystemB>
	constexpr static bool World<TickRate, SystemsSet, ComponentsSet>::orderAfter() {
		return SystemManager::orderAfter<SystemA, SystemB>();
	}
}
