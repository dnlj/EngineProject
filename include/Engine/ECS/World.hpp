#pragma once

// STD
#include <tuple>

// Engine
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/SystemManager.hpp>
#include <Engine/ECS/ComponentManager.hpp>
#include <Engine/ECS/EntityManager.hpp>
#include <Engine/ECS/FilterManager.hpp>


namespace Engine::ECS {
	/**
	 * @tparam SystemsSet The systems for this world to have.
	 * @tparam ComponentsSet The components for this world to have.
	 */
	template<class SystemsSet, class ComponentsSet>
	class World
		: private EntityManager
		, private ComponentManager<ComponentsSet>
		, private FilterManager
		, private SystemManager<SystemsSet> {

		public:
			// EntityManager members
			using EntityManager::isAlive;
			using EntityManager::getEntities;
			using EntityManager::setEnabled;
			using EntityManager::isEnabled;

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
			Entity createEntity(bool forceNew = false);

			/**
			 * @copydoc EntityManager::destroyEntity
			 */
			void destroyEntity(Entity ent);

			/**
			 * Adds a component to an entity.
			 * @param[in] ent The entity.
			 * @tparam Component The component.
			 * @return A reference to the added component.
			 */
			template<class Component>
			Component& addComponent(Entity ent);

			/**
			 * Adds components to an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the added components.
			 */
			template<class... Components>
			std::tuple<Components&...> addComponents(Entity ent);

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @param[in] cid The id of the component.
			 * @return True if the entity has the component; otherwise false.
			 */
			bool hasComponent(Entity ent, ComponentID cid);

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @tparam Component The component type.
			 * @return True if the entity has the component; otherwise false.
			 */
			template<class Component>
			bool hasComponent(Entity ent);

			/**
			 * Checks if an entity has components.
			 * @param[in] ent The entity.
			 * @param[in] cbits The bitset of components.
			 * @return True if the entity has the components; otherwise false.
			 */
			bool hasComponents(Entity ent, ComponentBitset cbits);

			/**
			 * Checks if an entity has the components.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return True if the entity has the components; otherwise false.
			 */
			template<class... Components>
			bool hasComponents(Entity ent);

			/**
			 * Removes a component from an entity.
			 * @param[in] ent The entity.
			 * @tparam Component The component.
			 */
			template<class Component>
			void removeComponent(Entity ent);

			/**
			 * Removes components from an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 */
			template<class... Components>
			void removeComponents(Entity ent);

			/**
			 * Gets a reference the component associated with an entity.
			 * @param[in] ent The entity.
			 * @tparam Component The component.
			 * @return A reference to the component.
			 */
			template<class Component>
			Component& getComponent(Entity ent);

			/**
			 * Gets a reference the components associated with an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the components.
			 */
			template<class... Components>
			std::tuple<Components&...> getComponents(Entity ent);

			/**
			 * Gets the components bitset for an entity.
			 * @param[in] ent The entity.
			 * @return The components bitset for the entity
			 */
			ComponentBitset getComponentsBitset(Entity ent) const;

			// TODO: Doc
			template<class... Components>
			EntityFilter& getFilterFor();

			template<>
			EntityFilter& getFilterFor() = delete;
	};
}

#include <Engine/ECS/World.ipp>
