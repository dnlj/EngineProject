#pragma once

namespace Engine::ECS {
	/** The maximum number of components registrable. Ideally this would be exactly the number of components used. */
	constexpr size_t MAX_COMPONENTS = 64;

	/** The type to use for entity ids. */
	using EntityID = size_t;

	/** The type to use for component ids. */
	using ComponentID = size_t;

	/** The bitset type used for storing what components an entity has */
	using ComponentBitset = std::bitset<MAX_COMPONENTS>;
}