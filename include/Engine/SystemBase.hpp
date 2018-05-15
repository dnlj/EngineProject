#pragma once

// STD
#include <vector>

// Engine
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/SystemManager.hpp>
#include <Engine/ECS/World.hpp>


namespace Engine {
	// TODO: Doc tparam
	/**
	 * @brief A base class from systems.
	 * This class provides functions for handling the addition and remove of entities that are suitable for most systems.
	 */
	template<class World>
	class SystemBase;

	/** @see SystemBase */
	template<class SystemsSet, class ComponentsSet>
	class SystemBase<ECS::World<SystemsSet, ComponentsSet>> : public Engine::ECS::System {
		public:
			// TODO: Doc
			using World = ECS::World<SystemsSet, ComponentsSet>;

			// TODO: Doc
			SystemBase(World& world);

			/**
			 * @brief Called by Engine::ECS when a entity is created.
			 * @param[in] ent The entity.
			 */
			virtual void onEntityCreated(ECS::EntityID eid) override;

			/**
			 * @brief Called by Engine::ECS when a component is added to an entity.
			 * @param[in] ent The entity.
			 * @param[in] cid The id of the component.
			 */
			virtual void onComponentAdded(ECS::EntityID eid, ECS::ComponentID cid) override;

			/**
			 * @brief Called by Engine::ECS when a component is removed from an entity.
			 * @param[in] ent The entity.
			 * @param[in] cid The id of the component.
			 */
			virtual void onComponentRemoved(ECS::EntityID eid, ECS::ComponentID cid) override;

			/**
			 * @brief Called by Engine::ECS when a entity is destroyed.
			 * @param[in] ent The entity.
			 */
			virtual void onEntityDestroyed(ECS::EntityID eid) override;

		protected:
			/** A sorted vector of all the entities processed by this system. */
			std::vector<Engine::ECS::EntityID> entities;

			/** An entity id indexed vector of entities used by this system. */
			std::vector<uint8_t> hasEntities;

			/** The bitset of components used by this system. */
			Engine::ECS::ComponentBitset cbits;

			// TODO: Doc
			World& world;

			/**
			 * @brief Adds an entity to the system.
			 * @param[in] ent The entity.
			 */
			void addEntity(ECS::EntityID eid);

			/**
			 * @brief Removes an entity from the system.
			 * @param[in] ent The entity.
			 */
			void removeEntity(ECS::EntityID eid);

			/**
			 * @brief Checks if the system has an entity.
			 * @param[in] ent The entity.
			 */
			bool hasEntity(ECS::EntityID eid);
	};
}

#include <Engine/SystemBase.ipp>
