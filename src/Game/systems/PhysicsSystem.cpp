// Game
#include <Game/World.hpp>
#include <Game/Math.hpp>
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
			auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ent);
			auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(ent);
			physProxyComp.apply(*physBodyComp.body);
		}
	}

	void PhysicsSystem::tick() {
		Engine::Clock::TimePoint interpTime;
		int buffSize = 0;
		Engine::Clock::Duration ping = {};

		for (const auto& ply : world.getFilter<ActionComponent, ConnectionComponent>()) {
			const auto& connComp = world.getComponent<ConnectionComponent>(ply);
			const auto& actComp = world.getComponent<ActionComponent>(ply); 
			buffSize = static_cast<int>(actComp.estBufferSize) + 1;
			ping = connComp.conn->getPing() + connComp.conn->getJitter();
			break;
		}

		for (const auto ent : world.getFilter<Filter>()) {
			//if (world.hasComponent<RemotePhysicsFlag>(ent)) {
			// TODO: rm. use RemotePhysicsFlag
			if (world.hasComponent<PhysicsInterpComponent>(ent) && world.getComponent<PhysicsInterpComponent>(ent).onlyUserVerified) {
				auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(ent);

				//ENGINE_DEBUG_ASSERT(physBodyComp.getBody().GetType() == b2_staticBody,
				//	"Incorrect body type for a remote physics entity"
				//);

				auto& physInterpComp = world.getComponent<PhysicsInterpComponent>(ent);
				physInterpComp.nextTime = {};
				physInterpComp.prevTime = {};

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// Explanation for time offset calculation
				//
				//
				// Total = DeJitter + RTT + NetRate + TickInterval + InputBuffer
				// DeJitterBuffer is a user tuneable variable. Smaller is more responsive but larger is more resilient to network condition variations.
				//
				//              DeJitterBuffer >      . RecvUpdate                            . ClientTick
				// Client: <-------------------|------|---------------------------------------|---------> Time
				//                          RTT/2 ---> \                                     /  <--- RTT/2
				//                                      \                                   /             
				// Server: <-----------------------------|----------|.....|----------------|------------>
				//                               NetRate >          >     < ServerTick     < InputBuffer
				//                                                  . ServerTick Complete
				//
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				constexpr auto dejitter = World::getTickInterval(); // TODO: make cvar
				constexpr auto netrate = std::chrono::milliseconds{51}; // TODO: dont hardcode. cvar/cmdlline
				constexpr auto serverTickTime = World::getTickInterval();
				const auto step = dejitter + ping + netrate + serverTickTime + World::getTickInterval() * buffSize;
				interpTime = world.getTickTime() - step;

				// TODO: this isnt great on the ECS/snapshot memory layout
				for (Engine::ECS::Tick t = world.getTick(); t > world.getTick() - tickrate; --t) {
					const auto& physProxyComp2 = world.getComponent<PhysicsProxyComponent>(ent, t);
					if (physProxyComp2.rollbackOverride) {
						const auto tickTime = world.getTickTime(t);
						if (tickTime >= interpTime) {
							physInterpComp.nextTrans = physProxyComp2.trans;
							physInterpComp.nextTime = tickTime;
						} else {
							physInterpComp.prevTrans = physProxyComp2.trans;
							physInterpComp.prevTime = tickTime;
							break;
						}
					}
				}

				constexpr auto zero = Engine::Clock::TimePoint{};
				if (physInterpComp.nextTime == zero) {
					ENGINE_WARN("nextTrans not found! ", world.getTick());
					if (physInterpComp.prevTime != zero) {
						physInterpComp.nextTrans = physInterpComp.prevTrans;
						physInterpComp.nextTime = physInterpComp.prevTime;
					} else {
						continue;
					}
				}

				if (physInterpComp.prevTime == zero) {
					ENGINE_WARN("prevTrans not found!");
					if (physInterpComp.nextTime != zero) {
						physInterpComp.prevTrans = physInterpComp.nextTrans;
						physInterpComp.prevTime = physInterpComp.nextTime;
					} else {
						continue;
					}
				}

				const auto i = static_cast<float32>(std::min(1.0, std::max(0.0, physInterpComp.calcInterpValue(interpTime))));
				auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ent);
				physBodyComp.getBody().SetTransform(
					Math::lerp(physInterpComp.nextTrans.p, physInterpComp.prevTrans.p, i),
					0
				);
				// TODO: ang, vel, etc. although vel doesnt make sense since this is a static body.
			}
		}

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
		for (const auto ent : world.getFilter<Filter>()) {
			auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(ent);
			physProxyComp.snap = false;
			physProxyComp.rollbackOverride = false;

			if (world.isPerformingRollback()) {
				const auto tick = world.getTick();
				if (world.hasComponent(ent, tick)) {
					const auto& physProxyComp2 = world.getComponent<PhysicsProxyComponent>(ent, tick);
					if (physProxyComp2.rollbackOverride) {
						physProxyComp = physProxyComp2;
					}
				}
			}
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
