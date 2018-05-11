#pragma once

// STD
#include <array>
#include <type_traits>

// Engine
#include <Engine/ECS/Common.hpp>

// Meta
#include <Meta/TypeSet/MakeUnique.hpp>


namespace Engine::ECS {
	// TODO: Doc
	// TODO: Move
	class System {
		public:
			// TODO: Make this use a typeset instead?
			// TODO: Ideally this would be const/static
			/** The bitset of systems to have higher priority than. */
			Engine::ECS::SystemBitset priorityBefore;

			// TODO: Make this use a typeset instead?
			// TODO: Ideally this would be const/static
			/** The bitset of systems to have lower priority than. */
			Engine::ECS::SystemBitset priorityAfter;

			virtual ~System() {}; // TODO: make pure virtual
			virtual void onEntityCreated(EntityID eid) {};
			virtual void onEntityDestroyed(EntityID eid) {};
			virtual void onComponentAdded(EntityID eid, ComponentID cid) {};
			virtual void onComponentRemoved(EntityID eid, ComponentID cid) {};
			virtual void run(float dt) {};
	};
}

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
			 * @brief Constructor.
			 */
			SystemManager();

			/**
			 * @brief Destructor.
			 */
			~SystemManager();

			/**
			 * @brief Get the id associated with a system.
			 * @tparam System The system.
			 * @return The id associated with @p System.
			 */
			template<class System>
			constexpr SystemID getSystemID() noexcept;

			/**
			 * @brief Gets a reference to the system.
			 * @tparam System The type of the system.
			 * @return A reference to the system.
			 */
			template<class System>
			System& getSystem();

			/**
			 * @brief Gets the bitset with the bits that correspond to the ids of the systems set.
			 * @tparam System1 The first system.
			 * @tparam System2 The second system.
			 * @tparam SystemN The third through nth systems.
			 */
			template<class System1, class System2, class... SystemN>
			SystemBitset getBitsetForSystems();

			/** @see getBitsetForSystems */
			template<class System1>
			SystemBitset getBitsetForSystems();

			/**
			 * @brief Runs `onEntityCreated` member function on all systems.
			 * @param[in] eid The id of the entity being created.
			 */
			void onEntityCreated(EntityID eid);

			/**
			 * @brief Runs `onComponentAdded` member function on all systems.
			 * @param[in] eid The id of the entity being added to.
			 * @param[in] cid The id of the component being added.
			 */
			void onComponentAdded(EntityID eid, ComponentID cid);

			/**
			 * @brief Runs `onComponentRemoved` member function on all systems.
			 * @param[in] eid The id of the entity being removed from.
			 * @param[in] cid The id of the component being removed.
			 */
			void onComponentRemoved(EntityID eid, ComponentID cid);

			/**
			 * @brief Runs the `onEntityDestroyed` member function on all systems.
			 * @param[in] eid The id of the entity being destroyed.
			 */
			void onEntityDestroyed(EntityID eid);

			/**
			 * @brief Runs the `run` member function on all systems.
			 * @param[in] dt The time delta between calls.
			 */
			void run(float dt);

		private:
			/** The number of systems used by this manager */
			constexpr static size_t count = sizeof...(Systems);

			/** The array used for storing system instances */
			std::array<System*, count> systems = {};

			/** The array used for storing system priorities */
			std::array<SystemBitset, count> priority = {};

			/** The order that the systems should be operated on based on priorities */
			std::array<SystemID, count> systemOrder;

			/**
			 * @brief Sorts all systems.
			 */
			void sort();
	};
}

#include <Engine/ECS/SystemManager.ipp>
