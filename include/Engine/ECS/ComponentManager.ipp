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
}
