#pragma once

// Engine
#include <Engine/ECS/ComponentManager.hpp>

// Static members
namespace Engine::ECS {
	template<class Component>
	ComponentID ComponentManager::getGlobalComponentID() {
		const static Component id = getNextGlobalComponentID();
		return id;
	}
}

namespace Engine::ECS {
	template<class Component>
	ComponentID ComponentManager::getComponentID() {
		return getComponentID(getGlobalComponentID<Component>());
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
}
