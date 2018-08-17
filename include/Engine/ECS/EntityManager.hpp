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
			using EntityContainer = std::vector<decltype(Entity::gen)>;

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
			bool isAlive(Entity ent) const;

			// TODO: Doc
			// TODO: Test
			void setEnabled(Entity ent, bool enabled);

			// TODO: Doc
			// TODO: Test
			bool isEnabled(Entity ent) const;

			// TODO: Doc
			// TODO: Test
			const EntityContainer& getEntities() const;

		private:
			EntityContainer aliveEntities;
			std::vector<uint8_t> enabledEntities;
			std::vector<Entity> deadEntities;
	};
}
