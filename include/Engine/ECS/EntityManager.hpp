#pragma once

// STD
#include <vector>

// Engine
#include <Engine/ECS/Common.hpp>


namespace Engine::ECS {
	/**
	 * Manages entities.
	 */
	class EntityManager {
		public:
			/**
			 * Creates a new entity.
			 * @param[in] forceNew When set to true, prevents the reuse of ids.
			 * @return The new entity.
			 */
			Entity createEntity(bool forceNew = false);

			/**
			 * Destroys an entity.
			 * @param[in] ent The entity.
			 */
			void destroyEntity(Entity ent);

			/**
			 * Checks is an entity is alive.
			 * @param[in] ent The entity.
			 * @return True if the entity is alive; otherwise false.
			 */
			bool isAlive(Entity ent);

		private:
			std::vector<decltype(Entity::gen)> aliveEntities;
			std::vector<Entity> deadEntities;
	};
}
