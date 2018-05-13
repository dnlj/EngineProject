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
	class World;

	/** @see World */
	template<
		template<class...> class SystemsType,
		class... Systems,
		template<class...> class ComponentsType,
		class... Components
	> class World<SystemsType<Systems...>, ComponentsType<Components...>>
		: private SystemManager<SystemsType<Systems...>>
		, private ComponentManager<ComponentsType<Components...>>
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
