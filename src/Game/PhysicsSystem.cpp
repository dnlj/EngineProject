// Game
#include <Game/PhysicsSystem.hpp>

namespace Game {
	PhysicsSystem::PhysicsSystem(World& world)
		: SystemBase{world}
		, physWorld{b2Vec2_zero}
		, contactListener{*this} {

		physWorld.SetContactListener(&contactListener);

		#if defined(DEBUG_PHYSICS)
			debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
			physWorld.SetDebugDraw(&debugDraw);
		#endif
	}

	void PhysicsSystem::run(float dt) {
		physWorld.Step(dt, 8, 3);

		#if defined(DEBUG_PHYSICS)
			debugDraw.reset();
			physWorld.DrawDebugData();
		#endif
	}

	b2Body* PhysicsSystem::createBody(Engine::ECS::Entity ent, b2BodyDef& bodyDef) {
		auto body = physWorld.CreateBody(&bodyDef);

		if (userData.size() <= ent.id) {
			userData.resize(ent.id + 1);
		}

		userData[ent.id] = PhysicsUserData{ent};
		body->SetUserData(reinterpret_cast<void*>(static_cast<std::uintptr_t>(ent.id)));

		return body;
	}

	const PhysicsUserData& PhysicsSystem::getUserData(void* ptr) const {
		return userData[reinterpret_cast<std::size_t>(ptr)];
	}

	#if defined(DEBUG_PHYSICS)
		Engine::Debug::DebugDrawBox2D& PhysicsSystem::getDebugDraw() {
			constexpr size_t a = sizeof(Engine::Debug::DebugDrawBox2D);
			return debugDraw;
		}
	#endif
}

namespace Game {
	PhysicsSystem::ContactListener::ContactListener(PhysicsSystem& physSys)
		: physSys{physSys} {
	}

	void PhysicsSystem::ContactListener::BeginContact(b2Contact* contact) {
		const auto dataA = static_cast<PhysicsUserData*>(contact->GetFixtureA()->GetBody()->GetUserData());
		const auto dataB = static_cast<PhysicsUserData*>(contact->GetFixtureB()->GetBody()->GetUserData());

		std::cout
			<< "A: "<< physSys.getUserData(dataA).ent << " "
			<< "B: " << physSys.getUserData(dataB).ent << "\n";
	}
}
