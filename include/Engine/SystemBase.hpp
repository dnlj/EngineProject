#pragma once

// STD
#include <vector>

// Engine
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/SystemManager.hpp>
#include <Engine/ECS/World.hpp>


namespace Engine {
	/**
	 * A base class from systems.
	 * This class provides functions for handling the addition and remove of entities that are suitable for most systems.
	 * @tparam World The world type.
	 */
	template<class World>
	class SystemBase;

	/** @see SystemBase */
	template<class SystemsSet, class ComponentsSet>
	class SystemBase<ECS::World<SystemsSet, ComponentsSet>> : public Engine::ECS::System {
		public:
			/** The world type used by this system. */
			using World = ECS::World<SystemsSet, ComponentsSet>;

			/**
			 * Constructor.
			 * @param[in,out] world The world that owns this system.
			 */
			SystemBase(World& world);

			/**
			 * Called by Engine::ECS when a entity is created.
			 * @param[in] eid The entity.
			 */
			virtual void onEntityCreated(ECS::EntityID eid) override;

			/**
			 * Called by Engine::ECS when a component is added to an entity.
			 * @param[in] eid The entity.
			 * @param[in] cid The id of the component.
			 */
			virtual void onComponentAdded(ECS::EntityID eid, ECS::ComponentID cid) override;

			/**
			 * Called by Engine::ECS when a component is removed from an entity.
			 * @param[in] eid The entity.
			 * @param[in] cid The id of the component.
			 */
			virtual void onComponentRemoved(ECS::EntityID eid, ECS::ComponentID cid) override;

			/**
			 * Called by Engine::ECS when a entity is destroyed.
			 * @param[in] eid The entity.
			 */
			virtual void onEntityDestroyed(ECS::EntityID eid) override;

		protected:
			/** A sorted vector of all the entities processed by this system. */
			std::vector<Engine::ECS::EntityID> entities;

			/** An entity id indexed vector of entities used by this system. */
			std::vector<uint8_t> hasEntities;

			/** The bitset of components used by this system. */
			Engine::ECS::ComponentBitset cbits;

			/** The world that owns this system. */
			World& world;

			/**
			 * Adds an entity to the system.
			 * @param[in] eid The entity.
			 */
			void addEntity(ECS::EntityID eid);

			/**
			 * Removes an entity from the system.
			 * @param[in] eid The entity.
			 */
			void removeEntity(ECS::EntityID eid);

			/**
			 * Checks if the system has an entity.
			 * @param[in] eid The entity.
			 */
			bool hasEntity(ECS::EntityID eid);

			/**
			 * Called when an entity is added to this system.
			 * @param[in] eid The id of the entity.
			 */
			virtual void onEntityAdded(ECS::EntityID eid);

			/**
			 * Called when an entity is removed from this system.
			 * @param[in] eid The id of the entity.
			 */
			virtual void onEntityRemoved(ECS::EntityID eid);
	};
}

#include <Engine/SystemBase.ipp>
