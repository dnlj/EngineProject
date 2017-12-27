#pragma once

// Engine
#include <Engine/Detail/Detail.hpp>
#include <Engine/Entity.hpp>

#define ENGINE_LOG(msg)\
	Engine::Detail::log(std::clog, "[LOG]", msg, __FILE__, __LINE__);

#define ENGINE_WARN(msg)\
	Engine::Detail::log(std::cerr, "[WARN]", msg, __FILE__, __LINE__);

#define ENGINE_ERROR(msg)\
	Engine::Detail::log(std::cerr, "[ERROR]", msg, __FILE__, __LINE__);\
	Engine::fatal();

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

	/**
	 * @brief Exits the program.
	 */
	void fatal();
}