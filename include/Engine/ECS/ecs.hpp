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

	/**
	 * The type used to store tick numbers.
	 * 
	 * Tick numbers are sequentially increasing and do _NOT_ wrap. Not wrapping
	 * should be safe, even for long running servers, because even at a high
	 * tick rate like 128 it would take over a year to exhaust the tick space:
	 *   @ 128 = (2^32)/(128*60*60*24) = 388 days
	 *   @ 64  = (2^32)/(64*60*60*24) = 776 days
	 * I don't anticipate any individual server will ever run that long.
	 */
	using Tick = uint32;

	/** The type to use for component ids. */
	using ComponentId = uint16;

	/** The type to use for system ids. */
	using SystemId = uint16;

	/** The bitset type used for storing what components an entity has. */
	using ComponentBitset = Bitset<MAX_COMPONENTS>; // TODO: is there a reason this isnt a member of World?

	/** The bitset type used for storing system priorites. */
	using SystemBitset = Bitset<MAX_SYSTEMS>; // TODO: is there a reason this isnt a member of World?

	using EntityStates = std::vector<EntityState>;
}

// Asserts
namespace Engine::ECS {
	static_assert(MAX_COMPONENTS < std::numeric_limits<ComponentId>::max(), "[Engine::ECS] MAX_COMPONENTS must not be larger than the maximum value for the type ComponentId.");
	static_assert(MAX_SYSTEMS <= std::numeric_limits<SystemId>::max(), "[Engine::ECS] MAX_SYSTEMS must not be greater than the maximum value for the type SystemId.");
}
