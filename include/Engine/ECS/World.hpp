#pragma once

// Engine
#include <Engine/ECS/SystemManager.hpp>
#include <Engine/ECS/ComponentManager.hpp>
#include <Engine/ECS/EntityManager.hpp>


namespace Engine::ECS {
	/**
	 * @tparam SystemsSet The systems for this world to have.
	 * @tparam ComponentsSet The components for this world to have.
	 */
	template<class SystemsSet, class ComponentsSet>
	class World
		: private SystemManager<SystemsSet>
		, private ComponentManager<ComponentsSet>
		, private EntityManager {

		public:
			// EntityManager members
			using EntityManager::createEntity;
			using EntityManager::destroyEntity;
			using EntityManager::isAlive;

			// ComponentManager members
			using ComponentManager::getComponentID;
			using ComponentManager::getBitsetForComponents;

			// SystemManager members
			using SystemManager::getSystemID;
			using SystemManager::getSystem;
			using SystemManager::getBitsetForSystems;
	};
}
