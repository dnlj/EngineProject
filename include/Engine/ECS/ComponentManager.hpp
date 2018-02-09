#pragma once

// STD
#include <array>
#include <unordered_map>

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
			ComponentManager();

			/**
			 * @brief Get the ComponentID associated with a component.
			 * @tparam Component The component.
			 * @return The id of @p Component.
			 */
			template<class Component>
			ComponentID getComponentID();

			/**
			 * @brief Get the ComponentID associated with @p name.
			 * @param[in] name The name of the component.
			 * @return The id of the component.
			 */
			ComponentID getComponentID(const std::string& name);

			/**
			 * @brief Gets the bitset with the bits that correspond to the ids of the components set.
			 * @tparam Component1 The first component.
			 * @tparam Component2 The second component.
			 * @tparam Components The third through nth component.
			 */
			template<class Component1, class Component2, class... Components>
			ComponentBitset getBitsetForComponents();

			/** @copydoc getBitsetForComponents */
			template<class Component>
			ComponentBitset getBitsetForComponents();

			/**
			 * @brief Registers a component.
			 * @param[in] name The name to associate with the component.
			 * @tparam Component The component.
			 */
			template<class Component>
			void registerComponent(const std::string name);

		private:
			/** The next id to use for components */
			ComponentID nextID = 0;

			/** The array used for translating from global to local ids */
			std::array<ComponentID, MAX_COMPONENTS> globalToLocalID;

			/** The map used for translating from strings to local ids */
			std::unordered_map<std::string, ComponentID> nameToID;

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