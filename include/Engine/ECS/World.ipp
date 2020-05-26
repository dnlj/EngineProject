#pragma once

// Engine
#include <Engine/ECS/World.hpp>


namespace Engine::ECS {
	WORLD_TPARAMS
	template<class Arg>
	WORLD_CLASS::World(float tickInterval, Arg& arg)
		: fm{em}
		, sm{arg}
		, beginTime{Clock::now()}
		, tickTime{beginTime} {
	}

	WORLD_TPARAMS
	void WORLD_CLASS::run() {
		const auto endTime = Clock::now();
		deltaTimeNS = endTime - beginTime;
		beginTime = endTime;
		deltaTime = Clock::Seconds{deltaTimeNS}.count();

		if (beginTime - tickTime > maxDelay) {
			ENGINE_WARN("World tick falling behind by ",
				Clock::Seconds{beginTime - tickTime - maxDelay}.count(), "s"
			);

			// We could instead limit the number of ticks in the while loop
			// which would have the effect of slowing down the world instead of
			// throwing away time like this does
			tickTime = beginTime - maxDelay;
		}
		
		while (tickTime + tickInterval <= beginTime) {
			constexpr auto tickDelta = Clock::Seconds{tickInterval}.count();
			sm.tick(tickDelta);
			tickTime += tickInterval;
		}

		sm.run(deltaTime);
	}

	WORLD_TPARAMS
	bool WORLD_CLASS::isAlive(Entity ent) const {
		return em.isAlive(ent);
	}

	WORLD_TPARAMS
	void WORLD_CLASS::setEnabled(Entity ent, bool enabled) {
		em.setEnabled(ent, enabled);
	}

	WORLD_TPARAMS
	bool WORLD_CLASS::isEnabled(Entity ent) const {
		return em.isEnabled(ent);
	}

	WORLD_TPARAMS
	const EntityManager::EntityContainer& WORLD_CLASS::getEntities() const {
		return em.getEntities();
	}

	WORLD_TPARAMS
	template<class... ComponentN>
	ComponentBitset WORLD_CLASS::getBitsetForComponents() const {
		return cm.getBitsetForComponents<ComponentN...>();
	}
	
	WORLD_TPARAMS
	template<class Component>
	constexpr static ComponentId WORLD_CLASS::getComponentId() noexcept {
		return ComponentManager::getComponentId<Component>();
	}

	WORLD_TPARAMS
	template<class System>
	constexpr static SystemId WORLD_CLASS::getSystemId() noexcept {
		return SystemManager::getSystemId<System>();
	}

	WORLD_TPARAMS
	template<class System>
	System& WORLD_CLASS::getSystem() {
		return sm.getSystem<System>();
	}

	WORLD_TPARAMS
	template<class... SystemN>
	SystemBitset WORLD_CLASS::getBitsetForSystems() const {
		return sm.getBitsetForSystems<SystemN...>();
	};

	WORLD_TPARAMS
	void WORLD_CLASS::run(float dt) {
		sm.run(dt);
	}

	WORLD_TPARAMS
	Entity WORLD_CLASS::createEntity(bool forceNew) {
		const auto ent = em.createEntity(forceNew);

		if (ent.id >= cm.componentBitsets.size()) {
			cm.componentBitsets.resize(ent.id + 1);
		} else {
			cm.componentBitsets[ent.id].reset();
		}

		return ent;
	}

	WORLD_TPARAMS
	void WORLD_CLASS::destroyEntity(Entity ent) {
		removeAllComponents(ent);
		em.destroyEntity(ent);
		fm.onEntityDestroyed(ent, cm.componentBitsets[ent.id]);
	}

	WORLD_TPARAMS
	template<class Component, class... Args>
	Component& WORLD_CLASS::addComponent(Entity ent, Args&&... args) {
		auto& container = cm.getComponentContainer<Component>();
		constexpr auto cid = getComponentId<Component>();
		container.add(ent.id, std::forward<Args>(args)...);

		// Add the component
		cm.componentBitsets[ent.id][cid] = true;

		// Update filters
		fm.onComponentAdded(ent, cid, cm.componentBitsets[ent.id]);

		return container[ent.id];
	}

	WORLD_TPARAMS
	template<class... Components>
	std::tuple<Components&...> WORLD_CLASS::addComponents(Entity ent) {
		return std::forward_as_tuple(addComponent<Components>(ent) ...);
	}

	WORLD_TPARAMS
	bool WORLD_CLASS::hasComponent(Entity ent, ComponentId cid) {
		return cm.componentBitsets[ent.id][cid];
	}

	WORLD_TPARAMS
	template<class Component>
	bool WORLD_CLASS::hasComponent(Entity ent) {
		return hasComponent(ent, getComponentId<Component>());
	}

	WORLD_TPARAMS
	bool WORLD_CLASS::hasComponents(Entity ent, ComponentBitset cbits) {
		return (cm.componentBitsets[ent.id] & cbits) == cbits;
	}

	WORLD_TPARAMS
	template<class... Components>
	bool WORLD_CLASS::hasComponents(Entity ent) {
		return hasComponents(ent, getBitsetForComponents<Components...>());
	}

	WORLD_TPARAMS
	template<class Component>
	void WORLD_CLASS::removeComponent(Entity ent) {
		removeComponents<Component>(ent);
	}

	WORLD_TPARAMS
	template<class... Components>
	void WORLD_CLASS::removeComponents(Entity ent) {
		cm.componentBitsets[ent.id] &= ~getBitsetForComponents<Components...>();

		(cm.getComponentContainer<Components>().remove(ent.id), ...);

		// TODO: Make filter manager take bitset?
		(fm.onComponentRemoved(ent, getComponentId<Components>()), ...);
	}
	
	WORLD_TPARAMS
	void WORLD_CLASS::removeAllComponents(Entity ent) {
		((hasComponent<Cs>(ent) && (removeComponent<Cs>(ent), 1)), ...);
	}

	WORLD_TPARAMS
	template<class Component>
	Component& WORLD_CLASS::getComponent(Entity ent) {
		return cm.getComponentContainer<Component>()[ent.id];
	}

	WORLD_TPARAMS
	template<class... Components>
	std::tuple<Components&...> WORLD_CLASS::getComponents(Entity ent) {
		return std::forward_as_tuple(getComponent<Components>(ent) ...);
	}

	WORLD_TPARAMS
	ComponentBitset WORLD_CLASS::getComponentsBitset(Entity ent) const {
		return cm.componentBitsets[ent.id];
	}

	WORLD_TPARAMS
	template<class... Components>
	EntityFilter& WORLD_CLASS::getFilterFor() {
		static_assert(sizeof...(Components) > 0, "Unable to get filter for no components.");
		return fm.getFilterFor(*this, getBitsetForComponents<Components...>());
	}
	
	WORLD_TPARAMS
	auto WORLD_CLASS::getTickInterval() const {
		return tickInterval;
	}
	
	WORLD_TPARAMS
	Clock::TimePoint WORLD_CLASS::getTickTime() const {
		return tickTime;
	}

	WORLD_TPARAMS
	float32 WORLD_CLASS::getTickRatio() const {
		return (beginTime - tickTime).count() / static_cast<float32>(tickInterval.count());
	}

	WORLD_TPARAMS
	float32 WORLD_CLASS::getDeltaTime() const {
		return deltaTime;
	}

	WORLD_TPARAMS
	auto WORLD_CLASS::getDeltaTimeNS() const {
		return deltaTimeNS;
	}

	WORLD_TPARAMS
	template<class SystemA, class SystemB>
	constexpr static bool WORLD_CLASS::orderBefore() {
		return SystemManager::orderBefore<SystemA, SystemB>();
	}

	WORLD_TPARAMS
	template<class SystemA, class SystemB>
	constexpr static bool WORLD_CLASS::orderAfter() {
		return SystemManager::orderAfter<SystemA, SystemB>();
	}
	
	WORLD_TPARAMS
	template<class Callable>
	void WORLD_CLASS::callWithComponent(Entity ent, ComponentId cid, Callable&& callable) {
		return cm.callWithComponent(ent, cid, std::forward<Callable>(callable));
	}
}
