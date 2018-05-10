#pragma once

// STD
#include <array>
#include <unordered_map>

// Engine
#include <Engine/ECS/Common.hpp>


// TODO: Document
// TODO: Test
namespace Engine::ECS {
	template<class ComponentsSet>
	class ComponentManager;

	template<template<class...> class ComponentsType, class... Components>
	class ComponentManager<ComponentsType<Components...>> {
		public:
			ComponentManager();

			/**
			 * @brief Get the ComponentID associated with a component.
			 * @tparam Component The component.
			 * @return The id of @p Component.
			 */
			template<class Component>
			constexpr ComponentID getComponentID() noexcept;

			/**
			 * @brief Gets the bitset with the bits that correspond to the ids of the components set.
			 * @tparam Component1 The first component.
			 * @tparam Component2 The second component.
			 * @tparam ComponentN The third through nth components.
			 */
			template<class Component1, class Component2, class... ComponentN>
			ComponentBitset getBitsetForComponents();

			/** @see getBitsetForComponents */
			template<class Component>
			ComponentBitset getBitsetForComponents();

		private:
	};
}

#include <Engine/ECS/ComponentManager.ipp>