#pragma once

// STD
#include <tuple>

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
			using EntityManager::isAlive;

			// ComponentManager members
			using ComponentManager::getComponentID;
			using ComponentManager::getBitsetForComponents;

			// SystemManager members
			using SystemManager::getSystemID;
			using SystemManager::getSystem;
			using SystemManager::getBitsetForSystems;
			using SystemManager::run;

		public:
			/**
			 * Constructor.
			 */
			World();

			/**
			 * @copydoc EntityManager::createEntity
			 */
			EntityID createEntity(bool forceNew = false);

			/**
			 * @copydoc EntityManager::destroyEntity
			 */
			void destroyEntity(EntityID eid);

			/**
			 * Adds a component to an entity.
			 * @param[in] eid The id of the entity.
			 * @tparam Component The component.
			 * @return A reference to the added component.
			 */
			template<class Component>
			Component& addComponent(EntityID eid);

			/**
			 * Adds components to an entity.
			 * @param[in] eid The id of the entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the added components.
			 */
			template<class... Components>
			std::tuple<Components&...> addComponents(EntityID eid);

			/**
			 * Checks if an entity has a component.
			 * @param[in] eid The id of the entity.
			 * @param[in] cid The id of the component.
			 * @return True if the entity has the component; otherwise false.
			 */
			bool hasComponent(EntityID eid, ComponentID cid);

			/**
			 * Checks if an entity has a component.
			 * @param[in] eid The id of the entity.
			 * @tparam Component The component type.
			 * @return True if the entity has the component; otherwise false.
			 */
			template<class Component>
			bool hasComponent(EntityID eid);

			/**
			 * Checks if an entity has components.
			 * @param[in] eid The id of the entity.
			 * @param[in] cbits The bitset of components.
			 * @return True if the entity has the components; otherwise false.
			 */
			bool hasComponents(EntityID eid, ComponentBitset cbits);

			/**
			 * Checks if an entity has the components.
			 * @param[in] eid The id of the entity.
			 * @tparam Components The components.
			 * @return True if the entity has the components; otherwise false.
			 */
			template<class... Components>
			bool hasComponents(EntityID eid);

			/**
			 * Removes a component from an entity.
			 * @param[in] eid The id of the entity.
			 * @tparam Component The component.
			 */
			template<class Component>
			void removeComponent(EntityID eid);

			/**
			 * Removes components from an entity.
			 * @param[in] eid The id of the entity.
			 * @tparam Components The components.
			 */
			template<class... Components>
			void removeComponents(EntityID eid);

			/**
			 * Gets a reference the component associated with an entity.
			 * @param[in] eid The id of the entity.
			 * @tparam Component The component.
			 * @return A reference to the component.
			 */
			template<class Component>
			Component& getComponent(EntityID eid);

			/**
			 * Gets a reference the components associated with an entity.
			 * @param[in] eid The id of the entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the components.
			 */
			template<class... Components>
			std::tuple<Components&...> getComponents(EntityID eid);

		private:
			/** The bitsets for storing what components entities have. */
			std::vector<ComponentBitset> componentBitsets;
	};
}

#include <Engine/ECS/World.ipp>
