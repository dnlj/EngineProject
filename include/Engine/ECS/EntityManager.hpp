#pragma once

// STD
#include <vector>

// Engine
#include <Engine/ECS/Common.hpp>

namespace Engine::ECS {
	class EntityManager {
		public:
			/**
			 * @brief Creates a new entity.
			 * @param[in] forceNew When set to true, prevents the reuse of ids.
			 * @return The id of the new entity.
			 */
			EntityID createEntity(bool forceNew = false);

		private:
			std::vector<ComponentBitset> entityComponents;
			std::vector<EntityID> deadEntities;
			std::vector<uint8_t> entityAlive;
	};
}