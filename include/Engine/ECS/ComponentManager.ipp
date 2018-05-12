#pragma once

// Engine
#include <Engine/ECS/ComponentManager.hpp>
#include <Engine/Engine.hpp>

// Meta
#include <Meta/IndexOf.hpp>

namespace Engine::ECS {
	template<template<class...> class ComponentsType, class... Components>
	ComponentManager<ComponentsType<Components...>>::ComponentManager() {
		// const auto id = getNextComponentID();
		// TODO: detail::ComponentData::addComponent[id] = addComponentToEntity<Component>;
		// TODO: detail::ComponentData::getComponent[id] = getComponentForEntity<Component>;
		// TODO: detail::ComponentData::reclaim[id] = reclaim<Component>;
	}

	template<template<class...> class ComponentsType, class... Components>
	template<class Component>
	constexpr ComponentID ComponentManager<ComponentsType<Components...>>::getComponentID() const noexcept {
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
