#pragma once

// Engine
#include <Engine/ECS/Entity.hpp>
#include <Engine/SparseSet.hpp>
#include <Engine/Bitset.hpp>
#include <Engine/Meta/IsComplete.hpp>
#include <Engine/ECS/EntityState.hpp>


namespace Engine::ECS {
	/** The maximum number of components registrable. Ideally this would be exactly the number of components used. */
	constexpr size_t MAX_COMPONENTS = 64;

	/** The maximum number of systems registrable. Ideally this would be exactly the number of systems used. */
	constexpr size_t MAX_SYSTEMS = 64;

	/** The type used to store tick numbers */
	using Tick = uint32;

	/** The type to use for component ids. */
	using ComponentId = uint16;

	/** The type to use for system ids. */
	using SystemId = uint16;

	/** The bitset type used for storing what components an entity has. */
	using ComponentBitset = Bitset<MAX_COMPONENTS>; // TODO: is there a reason this isnt a member of World?

	/** The bitset type used for storing system priorites. */
	using SystemBitset = Bitset<MAX_SYSTEMS>; // TODO: is there a reason this isnt a member of World?

	/** Determiens if a component is a flag component */ // TODO: doc flag components somewhere.
	template<class T, class = void>
	struct IsFlagComponent : std::true_type {};

	/** @see IsFlagComponent */
	template<class T>
	struct IsFlagComponent<T, std::enable_if_t<Meta::IsComplete<T>::value>> : std::is_empty<T> {};

	/** The stored data type for a given component */
	template<class T>
	using ComponentData = std::conditional_t<IsFlagComponent<T>::value, void, T>;

	/** The type used for storing components. */
	template<class T>
	struct ComponentContainer : SparseSet<Entity, ComponentData<T>> {};
	
	using EntityStates = std::vector<EntityState>;
}

// Asserts
namespace Engine::ECS {
	static_assert(MAX_COMPONENTS < std::numeric_limits<ComponentId>::max(), "[Engine::ECS] MAX_COMPONENTS must not be larger than the maximum value for the type ComponentId.");
	static_assert(MAX_SYSTEMS <= std::numeric_limits<SystemId>::max(), "[Engine::ECS] MAX_SYSTEMS must not be greater than the maximum value for the type SystemId.");
}
