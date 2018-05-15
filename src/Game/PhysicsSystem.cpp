// Game
#include <Game/PhysicsSystem.hpp>

namespace Game {
	PhysicsSystem::PhysicsSystem(World& world) : SystemBase{world} {
	}

	void PhysicsSystem::run(float dt) {
		puts("PhysicsSystem::run\n");
	}
}
