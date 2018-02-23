#pragma once

// STD
#include <bitset>

namespace Engine::ECS {
	/** The maximum number of components registrable. Ideally this would be exactly the number of components used. */
	constexpr size_t MAX_COMPONENTS = 64;

	/** The maximum number of systems registrable. Ideally this would be exactly the number of systems used. */
	constexpr size_t MAX_SYSTEMS = 64;

	/** The maximum number of systems registrable across the whole program. */
	constexpr size_t MAX_SYSTEMS_GLOBAL = 128;

	/** The type to use for entity ids. */
	using EntityID = size_t;

	/** The type to use for component ids. */
	using ComponentID = size_t;

	/** The type to use for system ids. */
	using SystemID = size_t;

	/** The bitset type used for storing what components an entity has */
	using ComponentBitset = std::bitset<MAX_COMPONENTS>;

	/** The bitset type used for storing system priorites */
	using SystemBitset = std::bitset<MAX_SYSTEMS>;
}
