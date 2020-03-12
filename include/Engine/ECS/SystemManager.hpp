#pragma once

// STD
#include <tuple>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/ECS/Common.hpp>

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

		// TODO: maybe add a concepts check?
		/*static_assert(
			std::conjunction<
				std::is_base_of<System, Systems>...
			>::value,
			"Each type must be a system."
		);*/

		private:
			/** The number of systems used by this manager. */
			constexpr static size_t count = sizeof...(Systems);

			/** How long between ticks (seconds). */
			const float32 tickInterval;

			/** The accumulator used to determine how many ticks to run. */
			float32 tickAccum = 0.0f;

			/** The systems to be managed. */
			std::tuple<Systems...> systems;

		public:
			/**
			 * Constructor.
			 * @param tickInterval How frequently (in seconds) to run system ticks.
			 * @param arg The argument to pass to system constructors.
			 */
			template<class Arg>
			SystemManager(float tickInterval, Arg& arg);

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
			constexpr static SystemID getSystemID() noexcept;

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
			 * Runs the `run` member function on all systems.
			 * @param[in] dt The time delta between calls.
			 */
			void run(float dt);
			
			/** Gets the tick interval in seconds. */
			float32 getTickInterval() const;

			/** Gets the remaining tick time to be simulated in seconds. */
			float32 getTickAccumulation() const;

		public:
			/**
			 * Returns true if @p SystemA will run before @p SystemB.
			 */
			template<class SystemA, class SystemB>
			constexpr static bool orderBefore();
			
			/**
			 * Returns true if @p SystemA will run after @p SystemB.
			 */
			template<class SystemA, class SystemB>
			constexpr static bool orderAfter();
	};
}

#include <Engine/ECS/SystemManager.ipp>
