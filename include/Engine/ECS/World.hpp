#pragma once

// Engine
#include <Engine/ECS/Common.hpp>
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
			using EntityManager::destroyEntity;
			using EntityManager::isAlive;

			// ComponentManager members
			using ComponentManager::getComponentID;
			using ComponentManager::getBitsetForComponents;

			// SystemManager members
			using SystemManager::getSystemID;
			using SystemManager::getSystem;
			using SystemManager::getBitsetForSystems;

		public:
			/**
			 * @copydoc EntityManager::createEntity
			 */
			EntityID createEntity(bool forceNew = false);

		private:
			// TODO: Doc
			std::vector<ComponentBitset> componentBitsets;
			
			/**
			 * @brief Get the bitset associated with an entity.
			 * @param[in] eid The id of the entity.
			 * @return A reference to the bitset associated with the entity.
			 */
			ComponentBitset& getComponentBitset(EntityID eid);

			/** @see getComponentBitset */
			const ComponentBitset& getComponentBitset(EntityID eid) const;
	};
}

#include <Engine/ECS/World.ipp>
