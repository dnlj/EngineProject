// Game
#include <Game/World.hpp>
#include <Game/PhysicsSystem.hpp>

namespace Game {
	PhysicsSystem::PhysicsSystem(SystemArg arg)
		: System{arg}
		, physWorld{b2Vec2_zero}
		, contactListener{*this}
		, filter{world.getFilterFor<PhysicsComponent>()} {

		physWorld.SetContactListener(&contactListener);

		#if defined(DEBUG_PHYSICS)
			debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
			physWorld.SetDebugDraw(&debugDraw);
		#endif
	}

	void PhysicsSystem::tick(float dt) {
		for (auto ent : filter) {
			auto& physComp = world.getComponent<PhysicsComponent>(ent);
			physComp.prevTransform = physComp.body->GetTransform();
		}

		physWorld.Step(dt, 8, 3);
	}

	void PhysicsSystem::run(float dt) {
		const float32 a = world.getTickRatio();
		const float32 b = 1.0f - a;

		for (auto ent : filter) {
			auto& physComp = world.getComponent<PhysicsComponent>(ent);
			const auto& prevTrans = physComp.prevTransform;
			const auto& nextTrans = physComp.body->GetTransform();
			auto& lerpTrans = physComp.interpTransform;

			lerpTrans.p = a * nextTrans.p + b * prevTrans.p;

			// Normalized lerp - really should use slerp but since the delta is small nlerp is close to slerp
			lerpTrans.q.c = a * nextTrans.q.c + b * prevTrans.q.c;
			lerpTrans.q.s = a * nextTrans.q.s + b * prevTrans.q.s;
			const float32 mag = lerpTrans.q.c * lerpTrans.q.c + lerpTrans.q.s * lerpTrans.q.s;
			lerpTrans.q.c /= mag;
			lerpTrans.q.s /= mag;
		}

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
		// TODO: change this to be whole ECS::Entity object
		body->SetUserData(reinterpret_cast<void*>(static_cast<std::uintptr_t>(ent.id)));

		return body;
	}

	void PhysicsSystem::destroyBody(b2Body* body) {
		physWorld.DestroyBody(body);
	}

	void PhysicsSystem::addListener(PhysicsListener* listener) {
		contactListener.addListener(listener);
	}

	void PhysicsSystem::shiftOrigin(const b2Vec2& newOrigin) {
		physWorld.ShiftOrigin(newOrigin);

		for (auto& ent : filter) {
			auto& physComp = world.getComponent<PhysicsComponent>(ent);
			physComp.prevTransform = physComp.body->GetTransform();
		}
	}

	const PhysicsUserData& PhysicsSystem::getUserData(const void* ptr) const {
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
		const auto dataA = contact->GetFixtureA()->GetBody()->GetUserData();
		const auto dataB = contact->GetFixtureB()->GetBody()->GetUserData();

		for (auto listener : listeners) {
			listener->beginContact(physSys.getUserData(dataA), physSys.getUserData(dataB));
		}
	}

	void PhysicsSystem::ContactListener::EndContact(b2Contact* contact) {
		const auto dataA = static_cast<PhysicsUserData*>(contact->GetFixtureA()->GetBody()->GetUserData());
		const auto dataB = static_cast<PhysicsUserData*>(contact->GetFixtureB()->GetBody()->GetUserData());

		for (auto listener : listeners) {
			listener->endContact(physSys.getUserData(dataA), physSys.getUserData(dataB));
		}
	}

	void PhysicsSystem::ContactListener::addListener(PhysicsListener* listener) {
		listeners.push_back(listener);
	}
}
