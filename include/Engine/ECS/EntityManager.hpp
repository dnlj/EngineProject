#pragma once

// STD
#include <vector>

// Engine
#include <Engine/ECS/Common.hpp>


// TODO: Document
// TODO: Test
namespace Engine::ECS {
	class EntityManager {
		public:
			/**
			 * Creates a new entity.
			 * @param[in] forceNew When set to true, prevents the reuse of ids.
			 * @return The id of the new entity.
			 */
			EntityID createEntity(bool forceNew = false);

			/**
			 * Destroys an entity.
			 * @param[in] eid The id of the entity.
			 */
			void destroyEntity(EntityID eid);

			/**
			 * Checks is an entity is alive.
			 * @param[in] eid The id of the entity.
			 * @return True if the entity is alive; otherwise false.
			 */
			bool isAlive(EntityID eid);

		private:
			std::vector<bool> aliveEntities;
			std::vector<EntityID> deadEntities;
	};
}