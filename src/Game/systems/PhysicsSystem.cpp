// Game
#include <Game/World.hpp>
#include <Game/systems/PhysicsSystem.hpp>

namespace {
	using Filter = Engine::ECS::EntityFilterList<
		Game::PhysicsComponent
	>;
}

namespace Game {
	PhysicsSystem::PhysicsSystem(SystemArg arg)
		: System{arg}
		, physWorld{b2Vec2_zero}
		, contactListener{*this} {

		physWorld.SetContactListener(&contactListener);

		#if defined(DEBUG_PHYSICS)
			debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
			physWorld.SetDebugDraw(&debugDraw);
		#endif
	}

	void PhysicsSystem::tick() {
		/*for (auto ent : world.getFilter<Filter>()) {
			auto& physComp = world.getComponent<PhysicsComponent>(ent);

			//if constexpr (ENGINE_CLIENT && false) { // TODO: split interp into own comp?
			//	if (!world.isNetworked(ent)) { continue; }
			//
			//	// TODO: angle
			//	const auto& pos = physComp.body->GetTransform().p;
			//	auto next = physComp.remoteTransform.p - pos;
			//	const auto len = next.Normalize();
			//	const float32 inc = 0.05f / (tickrate/5); // TODO: figure out good step size once world scale is fixed
			//	const float32 snap = 0.50f;
			//
			//	if (len <= 0.00001f) { // Close enough // TODO: FLT_epsilon?
			//		continue;
			//	} else if (len < inc || len >= snap) { // TODO: find good snap dist
			//		next = physComp.remoteTransform.p;
			//	} else {
			//		//ENGINE_LOG("Len: ", len, " (", physComp.remoteTransform.p.x, ", ", physComp.remoteTransform.p.y, ")");
			//		next = pos + (inc * next);
			//	}
			//
			//	physComp.updateTransform(next, 0);
			//}
		}*/
		
		// TODO: look into SetAutoClearForces
		physWorld.Step(world.getTickDelta(), 8, 3);
	}

	void PhysicsSystem::run(float dt) { // TODO: rm
		#if defined(DEBUG_PHYSICS)
			debugDraw.reset();
			physWorld.DrawDebugData();
		#endif
	}

	void PhysicsSystem::preStoreSnapshot() {
		for (auto ent : world.getFilter<Filter>()) {
			auto& physComp = world.getComponent<PhysicsComponent>(ent);
			physComp.snap = false;
			physComp.rollbackOverride = false;

			if (world.isPerformingRollback()) {
				const auto* snap = world.getSnapshot(world.getTick());
				if (!snap || !snap->hasComponent<PhysicsComponent>(ent)) { continue; }
				const auto& physComp2 = snap->getComponent<PhysicsComponent>(ent);
				if (physComp2.rollbackOverride) {
					physComp.stored = physComp2.stored;
					ENGINE_INFO("Rollback override!!!!!!!!"); // TODO: rm once done with testing
				}
			} else {
				physComp.storeBody();
			}
		}
	}

	void PhysicsSystem::postLoadSnapshot() {
		for (auto ent : world.getFilter<Filter>()) {
			auto& physComp = world.getComponent<PhysicsComponent>(ent);
			physComp.loadBody();
		}
	}

	b2Body* PhysicsSystem::createBody(Engine::ECS::Entity ent, b2BodyDef& bodyDef) {
		auto body = physWorld.CreateBody(&bodyDef);
		static_assert(sizeof(void*) >= sizeof(ent), "Engine::ECS::Entity is to large to store in userdata pointer.");
		body->SetUserData(reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t&>(ent)));
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

		for (auto& ent : world.getFilter<Filter>()) {
			auto& physComp = world.getComponent<PhysicsComponent>(ent);
			physComp.setTransform2(physComp.body->GetTransform());
		}
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
		const auto entA = toEntity(contact->GetFixtureA()->GetBody()->GetUserData());
		const auto entB = toEntity(contact->GetFixtureB()->GetBody()->GetUserData());

		for (auto listener : listeners) {
			listener->beginContact(entA, entB);
		}
	}

	void PhysicsSystem::ContactListener::EndContact(b2Contact* contact) {
		const auto entA = toEntity(contact->GetFixtureA()->GetBody()->GetUserData());
		const auto entB = toEntity(contact->GetFixtureB()->GetBody()->GetUserData());

		for (auto listener : listeners) {
			listener->endContact(entA, entB);
		}
	}

	void PhysicsSystem::ContactListener::addListener(PhysicsListener* listener) {
		listeners.push_back(listener);
	}
}
