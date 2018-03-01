#pragma once

// Engine
#include <Engine/ECS/ComponentManager.hpp>
#include <Engine/Engine.hpp>

// Static members
namespace Engine::ECS {
	template<class Component>
	ComponentID ComponentManager::getGlobalComponentID() {
		const static ComponentID id = getNextGlobalComponentID();
		return id;
	}
}

namespace Engine::ECS {
	template<class Component>
	ComponentID ComponentManager::getComponentID() {
		const auto cid = globalToLocalID[getGlobalComponentID<Component>()];

		#if defined(DEBUG)
			if (cid >= nextID) {
				ENGINE_ERROR("Attempting to get the local id of a nonregistered component.");
			}
		#endif

		return cid;
	}

	template<class Component>
	void ComponentManager::registerComponent(const std::string name) {
		const auto id = getNextComponentID();

		globalToLocalID[getGlobalComponentID<Component>()] = id;
		nameToID[std::move(name)] = id;

		// TODO: detail::ComponentData::addComponent[id] = addComponentToEntity<Component>;
		// TODO: detail::ComponentData::getComponent[id] = getComponentForEntity<Component>;
		// TODO: detail::ComponentData::reclaim[id] = reclaim<Component>;
	}

	template<class Component1, class Component2, class... Components>
	ComponentBitset ComponentManager::getBitsetForComponents() {
		return getBitsetForComponents<Component1>() |= getBitsetForComponents<Component2, Components...>();
	}

	template<class Component>
	ComponentBitset ComponentManager::getBitsetForComponents() {
		ComponentBitset value;
		value[getComponentID<Component>()] = true;
		return value;
	}
}
