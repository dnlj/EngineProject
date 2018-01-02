#pragma once

// STD
#include <vector>

// Engine
#include <Engine/Entity.hpp>

namespace Engine {

	/**
	 * @brief A base class from systems.
	 * This class provides functions for handling the addition and remove of entities that are suitable for most systems.
	 */
	class SystemBase {
		public:
			// TODO: Ideally this would be const/static
			/** The bitset of systems to have higher priority than. */
			Engine::ECS::SystemBitset priorityBefore;

			// TODO: Ideally this would be const/static
			/** The bitset of systems to have lower priority than. */
			Engine::ECS::SystemBitset priorityAfter;

			/**
			 * @brief Called by Engine::ECS when a entity is created.
			 * @param[in] ent The entity.
			 */
			void onEntityCreated(Entity ent);

			/**
			 * @brief Called by Engine::ECS when a component is added to an entity.
			 * @param[in] ent The entity.
			 */
			void onComponentAdded(Entity ent, ECS::ComponentID cid);

			/**
			 * @brief Called by Engine::ECS when a component is removed from an entity.
			 * @param[in] ent The entity.
			 */
			void onComponentRemoved(Entity ent, ECS::ComponentID cid);

			/**
			 * @brief Called by Engine::ECS when a entity is destroyed.
			 * @param[in] ent The entity.
			 */
			void onEntityDestroyed(Entity ent);

		protected:
			/** A sorted vector of all the entities processed by this system. */
			std::vector<Engine::Entity> entities;

			/** An entity id indexed vector of entities used by this system. */
			std::vector<uint8_t> hasEntities;

			/** The bitset of components used by this system. */
			Engine::ECS::ComponentBitset cbits;

			/**
			 * @brief Adds an entity to the system.
			 * @param[in] ent The entity.
			 */
			void addEntity(Entity ent);

			/**
			 * @brief Removes an entity from the system.
			 * @param[in] ent The entity.
			 */
			void removeEntity(Entity ent);

			/**
			 * @brief Checks if the system has an entity.
			 * @param[in] ent The entity.
			 */
			bool hasEntity(Entity ent);
	};
}