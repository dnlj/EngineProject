#pragma once

// STD
#include <array>

// Engine
#include <Engine/ECS/Common.hpp>

namespace Engine::ECS {
	// TODO: Doc
	// TODO: Move
	class System {
		public:
			// TODO: Ideally this would be const/static
			/** The bitset of systems to have higher priority than. */
			Engine::ECS::SystemBitset priorityBefore;

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
	class SystemManager {
		private:
			/**
			 * @brief Gets the next global id to use for systems.
			 * @return The next global id.
			 */
			static SystemID getNextGlobalSystemID();

			/**
			 * @brief Get the global id associated with a system.
			 * @tparam System The system.
			 * @return The global id of @p System.
			 */
			template<class System, class = std::enable_if_t<std::is_base_of_v<ECS::System, System>>>
			static SystemID getGlobalSystemID();

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
			SystemID getSystemID();

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
			 * @tparam Systems The third through nth systems.
			 */
			template<class System1, class System2, class... Systems>
			SystemBitset getBitsetForSystems();

			/** @copydoc getBitsetForSystems */
			template<class System>
			SystemBitset getBitsetForSystems();

			/**
			 * @brief Registers a system.
			 * @tparam System The system.
			 * @tparam Args the type of @p args.
			 * @param[in,out] args The arguments to foward to the constructor of @p System.
			 */
			template<class System, class... Args, class = std::enable_if_t<std::is_base_of_v<ECS::System, System>>>
			void registerSystem(Args&&... args);

			/**
			 * @brief Runs onEntityCreated member function on all registered systems.
			 * @param[in] eid The id of the entity being created.
			 */
			void onEntityCreated(EntityID eid);

			/**
			 * @brief Runs onComponentAdded member function on all registered systems.
			 * @param[in] eid The id of the entity being added to.
			 * @param[in] cid The id of the component being added.
			 */
			void onComponentAdded(EntityID eid, ComponentID cid);

			/**
			 * @brief Runs onComponentRemoved member function on all registered systems.
			 * @param[in] eid The id of the entity being removed from.
			 * @param[in] cid The id of the component being removed.
			 */
			void onComponentRemoved(EntityID eid, ComponentID cid);

			/**
			 * @brief Runs the onEntityDestroyed member function on all registered systems.
			 * @param[in] eid The id of the entity being destroyed.
			 */
			void onEntityDestroyed(EntityID eid);

			/**
			 * @brief Runs the run member function on all registered systems.
			 * @param[in] dt The time delta between calls.
			 */
			void run(float dt);

			/**
			 * @brief Sorts all registered systems.
			 */
			void sort();

		private:
			/** The next id to use for systems */
			SystemID nextID = 0;

			/** The array used for translating from global to local ids */
			std::array<SystemID, MAX_SYSTEMS_GLOBAL> globalToLocalID;

			using EntityModifyFunction = void(SystemManager::*)(EntityID);
			using ComponentModifyFunction = void(SystemManager::*)(EntityID, ComponentID);
			using RunFunction = void(SystemManager::*)(float);

			/** The number of registered systems */
			size_t count = 0;

			/** The array used for storing system instances */
			std::array<System*, MAX_SYSTEMS> systems = {};

			/** The array used for storing system priorities */
			std::array<SystemBitset, MAX_SYSTEMS> priority = {};

			/**
			 * @brief Gets the next id to use for systems.
			 * @return The next id.
			 */
			SystemID getNextSystemID();
	};
}

#include <Engine/ECS/SystemManager.ipp>
