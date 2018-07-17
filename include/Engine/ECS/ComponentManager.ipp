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
	constexpr static ComponentID ComponentManager<ComponentsType<Components...>>::getComponentID() noexcept {
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
		value[getComponentID<Component>()] = true;
		return value;
	}

	template<template<class...> class ComponentsType, class... Components>
	template<class Component>
	ComponentContainer<Component>& ComponentManager<ComponentsType<Components...>>::getComponentContainer() {
		return std::get<ComponentContainer<Component>>(containers);
	}
}
