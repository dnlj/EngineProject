#pragma once

// Meta
#include <Meta/IndexOf.hpp>

// Engine
#include <Engine/ECS/World.hpp>


namespace Engine::ECS {
	WORLD_TPARAMS
	template<class Arg>
	WORLD_CLASS::World(float tickInterval, Arg& arg)
		: fm{em}
		, systems((sizeof(Ss*), arg) ...)
		, beginTime{Clock::now()}
		, tickTime{beginTime} {

		(getSystem<Ss>().setup(), ...);
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
			(getSystem<Ss>().tick(tickDelta), ...);
			tickTime += tickInterval;
		}
		
		(getSystem<Ss>().run(deltaTime), ...);
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
		ComponentBitset value;
		((value[getComponentId<ComponentN>()] = true), ...);
		return value;
	}
	
	WORLD_TPARAMS
	template<class Component>
	constexpr static ComponentId WORLD_CLASS::getComponentId() noexcept {
		if constexpr ((std::is_same_v<Cs, Component> || ...)) {
			return Meta::IndexOf<Component, Cs...>::value;
		} else {
			return sizeof...(Cs) + Meta::IndexOf<Component, Fs...>::value;
		}
	}

	WORLD_TPARAMS
	template<class System>
	constexpr static SystemId WORLD_CLASS::getSystemId() noexcept {
		return Meta::IndexOf<System, Ss...>::value;
	}

	WORLD_TPARAMS
	template<class System>
	System& WORLD_CLASS::getSystem() {
		return std::get<System>(systems);
	}

	WORLD_TPARAMS
	template<class... SystemN>
	SystemBitset WORLD_CLASS::getBitsetForSystems() const {
		SystemBitset value;
		((value[getSystemId<SystemN>()] = true), ...);
		return value;
	};

	WORLD_TPARAMS
	Entity WORLD_CLASS::createEntity(bool forceNew) {
		const auto ent = em.createEntity(forceNew);

		if (ent.id >= compBitsets.size()) {
			compBitsets.resize(ent.id + 1); // TODO: Is one really the best increment size? Doesnt seem right.
		} else {
			compBitsets[ent.id].reset();
		}

		return ent;
	}

	WORLD_TPARAMS
	void WORLD_CLASS::destroyEntity(Entity ent) {
		removeAllComponents(ent);
		em.destroyEntity(ent);
		// TODO: shouldnt this be called before the entity is actually destroyed? bitset will be empty at this time.
		fm.onEntityDestroyed(ent, compBitsets[ent.id]);
	}

	WORLD_TPARAMS
	template<class F>
	constexpr static bool WORLD_CLASS::isFlag() {
		return (std::is_same_v<F, Fs> || ...);
	}

	WORLD_TPARAMS
	template<class C>
	constexpr static bool WORLD_CLASS::isComponent() {
		return (std::is_same_v<C, Cs> || ...);
	}

	WORLD_TPARAMS
	template<class Component, class... Args>
	Component& WORLD_CLASS::addComponent(Entity ent, Args&&... args) {
		constexpr auto cid = getComponentId<Component>();
		compBitsets[ent.id][cid] = true;
		fm.onComponentAdded(ent, cid, compBitsets[ent.id]);

		if constexpr (isComponent<Component>()) {
			auto& container = getComponentContainer<Component>();
			container.add(ent.id, std::forward<Args>(args)...);
			return container[ent.id];
		} else {
			// TODO: why is this not a compile error? this should need `decltype(auto)` return type?
			return compBitsets[ent.id][cid];
		}
	}

	WORLD_TPARAMS
	template<class... Components>
	std::tuple<Components&...> WORLD_CLASS::addComponents(Entity ent) {
		return std::forward_as_tuple(addComponent<Components>(ent) ...);
	}

	WORLD_TPARAMS
	bool WORLD_CLASS::hasComponent(Entity ent, ComponentId cid) {
		return compBitsets[ent.id][cid];
	}

	WORLD_TPARAMS
	template<class Component>
	bool WORLD_CLASS::hasComponent(Entity ent) {
		return hasComponent(ent, getComponentId<Component>());
	}

	WORLD_TPARAMS
	bool WORLD_CLASS::hasComponents(Entity ent, ComponentBitset cbits) {
		return (compBitsets[ent.id] & cbits) == cbits;
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
		compBitsets[ent.id] &= ~getBitsetForComponents<Components...>();

		((isComponent<Components>() && (getComponentContainer<Components>().remove(ent.id), true)), ...);

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
		// TODO: why is this not a compile error? this should need `decltype(auto)` return type?
		if constexpr (isFlag<Component>()) {
			return compBitsets[ent][getComponentId<Component>()];
		} else {
			return getComponentContainer<Component>()[ent.id];
		}
	}

	WORLD_TPARAMS
	template<class... Components>
	std::tuple<Components&...> WORLD_CLASS::getComponents(Entity ent) {
		return std::forward_as_tuple(getComponent<Components>(ent) ...);
	}

	WORLD_TPARAMS
	const ComponentBitset& WORLD_CLASS::getComponentsBitset(Entity ent) const {
		return compBitsets[ent.id];
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
		return Meta::IndexOf<SystemA, Ss...>::value < Meta::IndexOf<SystemB, Ss...>::value;
	}

	WORLD_TPARAMS
	template<class SystemA, class SystemB>
	constexpr static bool WORLD_CLASS::orderAfter() {
		return orderBefore<SystemB, SystemA>();
	}
	
	WORLD_TPARAMS
	template<class Callable>
	void WORLD_CLASS::callWithComponent(Entity ent, ComponentId cid, Callable&& callable) {
		// TODO: rm - using Caller = void(WORLD_CLASS::*)(Entity, Callable&&);
		using Caller = void(*)()
		constexpr auto callers[]{
			// TODO: rm - &WORLD_CLASS::template callWithComponentCaller<Cs, Callable>...
			&callable.operator()<Cs>...
		};
		//return (this->*callers[cid])(ent, std::forward<Callable>(callable));
		return callers[cid]();
	}

	WORLD_TPARAMS
	template<class Component>
	ComponentContainer<Component>& WORLD_CLASS::getComponentContainer() {
		return std::get<ComponentContainer<Component>>(compContainers);
	}
}
