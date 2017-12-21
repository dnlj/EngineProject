#pragma once

// STD
#include <ostream>

// Engine
#include <Engine/ECS/ECS.hpp>

namespace Engine {
	/**
	 * @brief A wrapper around the Engine::ECS entity system.
	 */
	class Entity {
		public:
			/**
			 * @brief Construct a new Entity with the entity id of @p eid.
			 * @param[in] eid The entity id to assign this entity.
			 */
			explicit Entity(ECS::EntityID eid);

			/**
			 * @brief Gets the EntityID assigned to this Entity.
			 * @return The EntityID assigned to this Entity.
			 */
			ECS::EntityID getID() const;

			/**
			 * @brief Adds a component of to this Entity.
			 * @param[in] cid The ComponentID for the type of component to add to this Entity.
			 */
			void addComponent(ECS::ComponentID cid);

			/**
			 * @copybrief Entity::addComponent
			 * @tparam The type of component to add to this Entity.
			 */
			template<class Component>
			void addComponent();

			/** 
			 * @brief Checks if an Entity has a component of type cid.
			 * @param[in] cid The ComponentID for the type of component to check for.
			 * @return True if this entity has the components; othewise false.
			 */
			bool hasComponent(ECS::ComponentID cid) const;

			/**
			 * @copybrief Entity::hasComponent
			 * @tparam The type of component to check for.
			 * @return True if this entity has the components; othewise false.
			 */
			template<class Component>
			bool hasComponent() const;

			/**
			 * @brief Checks if an Entity has components.
			 * @param[in] cbits The bitset of components.
			 * @return True if this entity has the components; othewise false.
			 */
			bool hasComponents(ECS::ComponentBitset cbits) const;

			/**
			 * @brief Removes a component from this Entity.
			 * @param[in] cid The ComponentID for the type of component to remove.
			 */
			void removeComponent(ECS::ComponentID cid);

			/**
			 * @copybrief Entity::removeComponent
			 * @tparam The type of component to remove.
			 */
			template<class Component>
			void removeComponent();

			/**
			 * @brief Gets a component from this Entity.
			 * @tparam The type of component to get.
			 * @return A reference to the component.
			 */
			template<class Component>
			Component& getComponent();

			/**
			 * @brief Checks if two entities are equal.
			 * @param[in] left The left operand.
			 * @param[in] right The right operand.
			 * @return True if @p left and @p right are equal.
			 */
			friend bool operator==(Entity left, Entity right);

			/**
			 * @brief Checks if two entities are not equal.
			 * @param[in] left The left operand.
			 * @param[in] right The right operand.
			 * @return True if @p left and @p right are not equal.
			 */
			friend bool operator!=(Entity left, Entity right);

			/**
			 * @brief Less than comparison.
			 * @param[in] left The left operand.
			 * @param[in] right The right operand.
			 * @return The result of the comparison.
			 */
			friend bool operator<(Entity left, Entity right);

			/**
			 * @brief Greater than comparison.
			 * @param[in] left The left operand.
			 * @param[in] right The right operand.
			 * @return The result of the comparison.
			 */
			friend bool operator>(Entity left, Entity right);

			/**
			 * @brief Less than or equal comparison.
			 * @param[in] left The left operand.
			 * @param[in] right The right operand.
			 * @return The result of the comparison.
			 */
			friend bool operator<=(Entity left, Entity right);

			/**
			 * @brief Greater than or equal comparison.
			 * @param[in] left The left operand.
			 * @param[in] right The right operand.
			 * @return The result of the comparison.
			 */
			friend bool operator>=(Entity left, Entity right);

			/**
			 * @brief Output an entity to @p os.
			 * @param[in] os The output stream.
			 * @param[in] ent The entity.
			 * @return @p os.
			 */
			friend std::ostream& operator<<(std::ostream& os, Entity ent);

		protected:
			/** The EntityID */
			const ECS::EntityID eid;
	};
}

#include <Engine/Entity.ipp>