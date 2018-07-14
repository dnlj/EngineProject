#pragma once

// STD
#include <array>
#include <type_traits>

// Engine
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/System.hpp>

// Meta
#include <Meta/TypeSet/MakeUnique.hpp>


namespace Engine::ECS {
	template<class SystemsSet>
	class SystemManager;

	template<template<class...> class SystemsType, class... Systems>
	class SystemManager<SystemsType<Systems...>> {
		static_assert(
			std::is_same<
				SystemsType<Systems...>,
				typename Meta::TypeSet::MakeUnique<SystemsType<Systems...>>::type
			>::value,
			"Each system must be unique."
		);
		
		static_assert(
			std::conjunction<
				std::is_base_of<System, Systems>...
			>::value,
			"Each type must be a system."
		);

		public:
			/**
			 * Constructor.
			 */
			template<class World>
			SystemManager(World& world);

			/**
			 *  Deleted copy constructor
			 */
			SystemManager(SystemManager&) = delete;

			/**
			 * Destructor.
			 */
			~SystemManager();

			/**
			 * Deleted assignment operator.
			 */
			SystemManager& operator=(const SystemManager&) = delete;

			/**
			 * Get the id associated with a system.
			 * @tparam System The system.
			 * @return The id associated with @p System.
			 */
			template<class System>
			constexpr SystemID getSystemID() const noexcept;

			/**
			 * Gets a reference to the system.
			 * @tparam System The type of the system.
			 * @return A reference to the system.
			 */
			template<class System>
			System& getSystem();

			/**
			 * Gets the bitset with the bits that correspond to the ids of the systems set.
			 * @tparam System1 The first system.
			 * @tparam System2 The second system.
			 * @tparam SystemN The third through nth systems.
			 */
			template<class System1, class System2, class... SystemN>
			SystemBitset getBitsetForSystems() const;

			/** @see getBitsetForSystems */
			template<class System1>
			SystemBitset getBitsetForSystems() const;

			/**
			 * Runs `onEntityCreated` member function on all systems.
			 * @param[in] ent The entity being created.
			 */
			void onEntityCreated(Entity ent);

			/**
			 * Runs `onComponentAdded` member function on all systems.
			 * @param[in] ent The entity being added to.
			 * @param[in] cid The id of the component being added.
			 */
			void onComponentAdded(Entity ent, ComponentID cid);

			/**
			 * Runs `onComponentRemoved` member function on all systems.
			 * @param[in] ent The entity being removed from.
			 * @param[in] cid The id of the component being removed.
			 */
			void onComponentRemoved(Entity ent, ComponentID cid);

			/**
			 * Runs the `onEntityDestroyed` member function on all systems.
			 * @param[in] ent The entity being destroyed.
			 */
			void onEntityDestroyed(Entity ent);

			/**
			 * Runs the `run` member function on all systems.
			 * @param[in] dt The time delta between calls.
			 */
			void run(float dt);

		private:
			/** The number of systems used by this manager. */
			constexpr static size_t count = sizeof...(Systems);

			/** The array used for storing system instances. */
			std::array<System*, count> systems = {};

			/** The array used for storing system priorities. */
			std::array<SystemBitset, count> priority = {};

			/** The order that the systems should be operated on based on priorities. */
			std::array<SystemID, count> systemOrder;

			/**
			 * Sorts all systems.
			 */
			void sort();
	};
}

#include <Engine/ECS/SystemManager.ipp>
