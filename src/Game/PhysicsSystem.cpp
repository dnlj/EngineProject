// Game
#include <Game/PhysicsSystem.hpp>

namespace Game {
	PhysicsSystem::PhysicsSystem(World& world)
		: SystemBase{world}
		, physWorld{b2Vec2_zero} {

		debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
		physWorld.SetDebugDraw(&debugDraw);
	}

	void PhysicsSystem::run(float dt) {
		physWorld.Step(dt, 8, 3);
		debugDraw.reset();
		physWorld.DrawDebugData();
		debugDraw.draw();
	}

	b2World& PhysicsSystem::getPhysicsWorld() {
		return physWorld;
	}
}
