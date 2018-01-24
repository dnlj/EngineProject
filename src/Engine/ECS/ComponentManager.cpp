// Engine
#include <Engine/ECS/ComponentManager.hpp>

namespace Engine::ECS {
	ComponentID ComponentManager::getNextComponentID() {
		return nextComponentID++;
	}
}