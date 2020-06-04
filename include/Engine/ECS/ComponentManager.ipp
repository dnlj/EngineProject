#pragma once

// Engine
#include <Engine/ECS/ComponentManager.hpp>
#include <Engine/Engine.hpp>

// Meta
#include <Meta/IndexOf.hpp>

namespace Engine::ECS {
	template<template<class...> class ComponentsType, class... Components>
	ComponentManager<ComponentsType<Components...>>::ComponentManager() {
	}

	template<template<class...> class ComponentsType, class... Components>
	template<class Component>
	constexpr static ComponentId ComponentManager<ComponentsType<Components...>>::getComponentId() noexcept {
		return Meta::IndexOf<Component, Components...>::value;
	}

	template<template<class...> class ComponentsType, class... Components>
	template<class Component1, class Component2, class... ComponentN>
	ComponentBitset ComponentManager<ComponentsType<Components...>>::getBitsetForComponents() const {
		return getBitsetForComponents<Component1>() |= (getBitsetForComponents<Component2>() |= ... |=  getBitsetForComponents<ComponentN>());
	}

	template<template<class...> class ComponentsType, class... Components>
	template<class Component>
	ComponentBitset ComponentManager<ComponentsType<Components...>>::getBitsetForComponents() const {
		ComponentBitset value;
		value[getComponentId<Component>()] = true;
		return value;
	}

	template<template<class...> class ComponentsType, class... Components>
	template<class Component>
	ComponentContainer<Component>& ComponentManager<ComponentsType<Components...>>::getComponentContainer() {
		return std::get<ComponentContainer<Component>>(containers);
	}
	
	template<template<class...> class ComponentsType, class... Components>
	template<class Callable>
	void ComponentManager<ComponentsType<Components...>>::callWithComponent(Entity ent, ComponentId cid, Callable&& callable) {
		using Caller = void(ComponentManager::*)(Entity, Callable&&);
		constexpr Caller callers[]{
			&ComponentManager<ComponentsType<Components...>>::template callWithComponentCaller<Components, Callable>...
		};
		return (this->*callers[cid])(ent, std::forward<Callable>(callable));
	}
	
	template<template<class...> class ComponentsType, class... Components>
	template<class Component, class Callable>
	void ComponentManager<ComponentsType<Components...>>::callWithComponentCaller(Entity ent, Callable&& callable) {
		return callable(static_cast<Component*>(nullptr));
	}
}
