#pragma once

// STD
#include <array>

// Engine
#include <Engine/ECS/Common.hpp>

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
			template<class System, class = std::enable_if_t<IsSystem<System>::value>>
			static SystemID getGlobalSystemID();

		public:
			/**
			 * @brief Constructor.
			 */
			SystemManager();

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
			template<class System, class... Args, class = std::enable_if_t<IsSystem<System>::value>>
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

			struct {
				using EntityModifyFunction = void(SystemManager::*)(EntityID);
				using ComponentModifyFunction = void(SystemManager::*)(EntityID, ComponentID);
				using RunFunction = void(SystemManager::*)(float);

				/** The number of registered systems */
				size_t count = 0;

				/** The array used for storing system instances */
				std::array<void*, MAX_SYSTEMS> system;

				/** The array used for storing system priorities */
				std::array<SystemBitset, MAX_SYSTEMS> priority;

				// TODO: Doc
				std::array<EntityModifyFunction, MAX_SYSTEMS> onEntityCreated;
				std::array<EntityModifyFunction, MAX_SYSTEMS> onEntityDestroyed;
				std::array<ComponentModifyFunction, MAX_SYSTEMS> onComponentAdded;
				std::array<ComponentModifyFunction, MAX_SYSTEMS> onComponentRemoved;
				std::array<RunFunction, MAX_SYSTEMS> run;
			} systems;

			/**
			 * @brief Gets the next id to use for systems.
			 * @return The next id.
			 */
			SystemID getNextSystemID();

			/**
			 * @brief Get the id associated with a global system id.
			 * @param[in] gsid The global id.
			 * @return The id associated with @p gsid.
			 */
			SystemID getSystemID(SystemID gsid);

			/**
			 * @brief Calls the onEntityCreated member function on the system.
			 * @tparam System The type of the system.
			 * @param[in] eid The id of the entity being created.
			 */
			template<class System>
			void onEntityCreatedCall(EntityID eid);

			/**
			 * @brief Calls the onEntityDestroyed member function on the system.
			 * @tparam System The type of the system.
			 * @param[in] eid The id of the entity being destroyed.
			 */
			template<class System>
			void onEntityDestroyedCall(EntityID eid);

			/**
			 * @brief Calls the onComponentAdded member function on the system.
			 * @tparam System The type of the system.
			 * @param[in] eid The id of the entity being added to.
			 * @param[in] cid The id of the component being added.
			 */
			template<class System>
			void onComponentAddedCall(EntityID eid, ComponentID cid);

			/**
			 * @brief Calls the onComponentRemoved member function on the system.
			 * @tparam System The type of the system.
			 * @param[in] eid The id of the entity being removed from.
			 * @param[in] cid The id of the component being removed.
			 */
			template<class System>
			void onComponentRemovedCall(EntityID eid, ComponentID cid);

			/**
			 * @brief Calls the run member function on the system.
			 * @tparam System The type of the system.
			 * @param[in] dt The time delta between calls.
			 */
			template<class System>
			void runCall(float dt);

	};
}

#include <Engine/ECS/SystemManager.ipp>
