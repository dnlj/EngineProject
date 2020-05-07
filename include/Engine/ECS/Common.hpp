#pragma once

// STD
#include <bitset>
#include <vector>

// Engine
#include <Engine/ECS/Entity.hpp>
#include <Engine/SparseSet.hpp>

namespace Engine::ECS {
	/** The maximum number of components registrable. Ideally this would be exactly the number of components used. */
	constexpr size_t MAX_COMPONENTS = 64;

	/** The maximum number of systems registrable. Ideally this would be exactly the number of systems used. */
	constexpr size_t MAX_SYSTEMS = 64;

	/** The type to use for component ids. */
	using ComponentId = uint16;

	/** The type to use for system ids. */
	using SystemId = uint16;

	/** The bitset type used for storing what components an entity has. */
	using ComponentBitset = std::bitset<MAX_COMPONENTS>;

	/** The bitset type used for storing system priorites. */
	using SystemBitset = std::bitset<MAX_SYSTEMS>;

	/** The type used for storing components. */
	template<class T>
	using ComponentContainer = SparseSet<decltype(Entity::id), T>;
}

// Asserts
namespace Engine::ECS {
	static_assert(MAX_COMPONENTS < std::numeric_limits<ComponentId>::max(), "[Engine::ECS] MAX_COMPONENTS must not be larger than the maximum value for the type ComponentId.");
	static_assert(MAX_SYSTEMS <= std::numeric_limits<SystemId>::max(), "[Engine::ECS] MAX_SYSTEMS must not be greater than the maximum value for the type SystemId.");
}
