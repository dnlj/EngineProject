// Game
#include <Game/PhysicsSystem.hpp>

namespace Game {
	void PhysicsSystem::run(float dt) {
		puts("PhysicsSystem::run\n");
	}

	ENGINE_REGISTER_SYSTEM(PhysicsSystem);
}
