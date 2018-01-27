#pragma once

// STD
#include <array>

// Engine
#include <Engine/ECS/Common.hpp>


// TODO: Document
// TODO: Test
namespace Engine::ECS {
	class ComponentManager {
		private:
			/**
			 * @brief Gets the next global id to use for components.
			 * @return The next id.
			 */
			static ComponentID getNextGlobalComponentID();

			/**
			 * @brief Get the global id associated with a component.
			 * @tparam Component The component.
			 * @return The global id of @p Component.
			 */
			template<class Component>
			static ComponentID getGlobalComponentID();

		public:
			/**
			 * @brief Get the ComponentID associated with a component.
			 * @tparam Component The component.
			 * @return The id of @p Component.
			 */
			template<class Component>
			ComponentID getComponentID();

		private:
			/** The next id to use for components */
			ComponentID nextID = 0;

			/** The array used for translating from global to local ids */
			std::array<ComponentID, MAX_COMPONENTS> globalToLocalID;

			/**
			 * @brief Gets the next id to use for components.
			 * @return The next id.
			 */
			ComponentID getNextComponentID();

			/**
			 * @brief Get the id associated with a global component id.
			 * @param gcid The global id.
			 * @return The id associated with 
			 */
			ComponentID getComponentID(ComponentID gcid);
	};
}

#include <Engine/ECS/ComponentManager.ipp>