#pragma once

// STD
#include <tuple>
#include <utility>
#include <type_traits>

// Engine
#include <Engine/ECS/Common.hpp>


namespace Engine::ECS {
	template<class ComponentsSet>
	class ComponentManager;

	template<template<class...> class ComponentsType, class... Components>
	class ComponentManager<ComponentsType<Components...>> {
		public:
			/**
			 * Constructor.
			 */
			ComponentManager();

			/**
			 * Get the ComponentID associated with a component.
			 * @tparam Component The component.
			 * @return The id of @p Component.
			 */
			template<class Component>
			constexpr static ComponentID getComponentID() noexcept;

			/**
			 * Gets the bitset with the bits that correspond to the ids of the components set.
			 * @tparam Component1 The first component.
			 * @tparam Component2 The second component.
			 * @tparam ComponentN The third through nth components.
			 */
			template<class Component1, class Component2, class... ComponentN>
			ComponentBitset getBitsetForComponents() const;

			/** @see getBitsetForComponents */
			template<class Component>
			ComponentBitset getBitsetForComponents() const;

			/**
			 * Get the container for components of type @p Component.
			 * @tparam Component The type of the component.
			 * @return A reference to the container associated with @p Component.
			 */
			template<class Component>
			ComponentContainer<Component>& getComponentContainer();

			/** The bitsets for storing what components entities have. */
			std::vector<ComponentBitset> componentBitsets;

			// TODO: This feels like it should be part of ECS::World (involves an Entity) but since that only has access to `ComponentsSet` and not `Components...` it cant.
			// TODO: Doc
			template<class Callable>
			void callWithComponent(Entity ent, ComponentID cid, Callable&& callable);

		private:
			// TODO: doc
			template<class Component, class Callable>
			void callWithComponentCaller(Entity ent, Callable&& callable);

			/** The number of components used by this manager. */
			constexpr static size_t count = sizeof...(Components);

			/** The containers for storing components. */
			std::tuple<ComponentContainer<Components>...> containers;
	};
}

#include <Engine/ECS/ComponentManager.ipp>
