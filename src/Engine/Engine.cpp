// Engine
#include <Engine/Engine.hpp>

namespace Engine {
	Entity createEntity(bool forceNew) {
		return Entity{Engine::ECS::createEntity(forceNew)};
	}

	void destroyEntity(Entity ent) {
		ECS::destroyEntity(ent.getID());
	}
}