#pragma once

// Engine
#include <Engine/Entity.hpp>

namespace Engine {
	/**
	 * @brief Creates a new Entity.
	 * @param[in] forceNew If set to true, disallows the reuse of old an Entity.
	 * @return A new Entity;
	 */
	Entity createEntity(bool forceNew = false);


	/**
	 * @brief Destroys a Entity.
	 * @param[in] ent The Entity to destroy.
	 */
	void destroyEntity(Entity ent);
}