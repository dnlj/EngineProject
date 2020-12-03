// Game
#include <Game/World.hpp>
#include <Game/systems/PhysicsSystem.hpp>

namespace {
	using Filter = Engine::ECS::EntityFilterList<
		Game::PhysicsBodyComponent
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

	void PhysicsSystem::preTick() {
		for (const auto ent : world.getFilter<Filter>()) {
			const auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ent);
			const auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(ent);

			physProxyComp.apply(*physBodyComp.body);
		}
	}

	void PhysicsSystem::tick() {
		// TODO: look into SetAutoClearForces
		physWorld.Step(world.getTickDelta(), 8, 3);
	}

	void PhysicsSystem::postTick() {
		for (const auto ent : world.getFilter<Filter>()) {
			const auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ent);
			auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(ent);

			physProxyComp.store(*physBodyComp.body);
		}
	}

	void PhysicsSystem::run(float dt) { // TODO: rm
		#if defined(DEBUG_PHYSICS)
			debugDraw.reset();
			physWorld.DrawDebugData();
		#endif
	}

	void PhysicsSystem::preStoreSnapshot() {
		auto& filter = world.getFilter<Filter>();
		for (const auto ent : filter) {
			auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(ent);
			physProxyComp.snap = false;
			physProxyComp.rollbackOverride = false;

			// TODO: rollback update
			//if (world.isPerformingRollback()) {
			//	const auto* snap = world.getSnapshot(world.getTick());
			//	if (!snap || !snap->hasComponent<PhysicsProxyComponent>(ent)) { continue; }
			//
			//	const auto& physProxyComp2 = snap->getComponent<PhysicsProxyComponent>(ent);
			//	if (physProxyComp2.rollbackOverride) {
			//		physProxyComp = physProxyComp2;
			//		ENGINE_INFO("Rollback override!!!!!!!!"); // TODO: rm once done with testing
			//	}
			//}
		}
	}

	void PhysicsSystem::postLoadSnapshot() {
		//for (auto ent : world.getFilter<Filter>()) {
		//	auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
		//	physComp.loadBody();
		//}
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
			auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
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
