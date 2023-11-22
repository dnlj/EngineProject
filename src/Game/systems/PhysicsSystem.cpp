// Game
#include <Game/World.hpp>
#include <Game/Math.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/systems/NetworkingSystem.hpp>

namespace {
	// TODO: rm or rename
	using ProxyFilter = Engine::ECS::EntityFilterList<
		Game::PhysicsBodyComponent
	>;

}

namespace Game {
	PhysicsSystem::PhysicsSystem(SystemArg arg)
		: System{arg}
		, physWorld{b2Vec2_zero} {

		physWorld.SetContactListener(this);
		physWorld.SetContactFilter(this);

		#if defined(DEBUG_PHYSICS)
			debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
			physWorld.SetDebugDraw(&debugDraw);
			debugDraw.setup(engine.getCamera());
		#endif
	}

	
	void PhysicsSystem::onComponentAdded(const Engine::ECS::Entity ent, PhysicsBodyComponent& comp) {
		// ENGINE_INFO(" PhysicsSystem - component added to ", ent);
	};

	void PhysicsSystem::onComponentRemoved(const Engine::ECS::Entity ent, PhysicsBodyComponent& comp) {
		// ENGINE_INFO(" PhysicsSystem - component removed from ", ent);
		physWorld.DestroyBody(comp.takeOwnership());
	};

	void PhysicsSystem::tick() {
		if constexpr (ENGINE_CLIENT || ENGINE_SERVER) { // TODO: this should be client only correct?
			Engine::Clock::TimePoint interpTime;
			int buffSize = 0; // TODO: should probably have est value initially
			Engine::Clock::Duration ping = {}; // TODO: should probably have est value initially

			// TODO: this is a bad way of handling this, we really need this per connection. Atm this might pick the server discover, an old DC'd connection, etc.
			// TODO: when we split the connection object, we really only need the stats part here for ping/jitter.
			for (const auto& ply : world.getFilter<ActionComponent>()) {
				const auto& actComp = world.getComponent<ActionComponent>(ply);
				auto* conn = world.getSystem<NetworkingSystem>().getConnection(ply);
				buffSize = static_cast<int>(actComp.estBufferSize) + 1;
				ping = conn->getPing() + conn->getJitter();
				break;
			}

			for (const auto ent : world.getFilter<PhysicsBodyComponent, PhysicsInterpComponent>()) {
				auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ent);
				auto& physInterpComp = world.getComponent<PhysicsInterpComponent>(ent);
				if (physInterpComp.onlyUserVerified && physBodyComp.getType() != b2_staticBody) {
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
						const auto& physCompState = world.getComponentState<PhysicsBodyComponent>(ent, t);
						if (physCompState.rollbackOverride) {
							const auto tickTime = world.getTickTime(t);
							if (tickTime >= interpTime) {
								physInterpComp.nextTrans = physCompState.trans;
								physInterpComp.nextTime = tickTime;
							} else {
								physInterpComp.prevTrans = physCompState.trans;
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
					physBodyComp.setTransform(
						Math::lerp(physInterpComp.nextTrans.p, physInterpComp.prevTrans.p, i),
						0
					);
					// TODO: ang, vel, etc. although vel doesnt make sense since this is a static body.
				}
			}
		}

		// TODO: look into SetAutoClearForces
		physWorld.Step(world.getTickDelta(), 8, 3);
	}

	void PhysicsSystem::render(const RenderLayer layer) {
		#if defined(DEBUG_PHYSICS)
		if (layer == RenderLayer::PhysicsDebug) {
			debugDraw.reset();
			physWorld.DrawDebugData();
			debugDraw.draw();
		}
		#endif
	}

	void PhysicsSystem::preStoreSnapshot() {
		// TODO: client only?
		for (const auto ent : world.getFilter<PhysicsBodyComponent>()) {
			auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
			physComp.snap = false;
			physComp.rollbackOverride = false;

			// If we already have a confirmed state use that.
			if (world.isPerformingRollback()) {
				const auto tick = world.getTick();
				if (world.hasComponent(ent, tick)) {
					const auto& physCompState2 = world.getComponentState<PhysicsBodyComponent>(ent, tick);
					if (physCompState2.rollbackOverride) {
						Engine::ECS::SnapshotTraits<PhysicsBodyComponent>::fromSnapshot(physComp, physCompState2);
					}
				}
			}
		}
	}

	b2Body* PhysicsSystem::createBody(Engine::ECS::Entity ent, b2BodyDef& bodyDef/*, PhysicsGroup group*/) {
		auto body = physWorld.CreateBody(&bodyDef);
		static_assert(sizeof(void*) >= sizeof(ent), "Engine::ECS::Entity is to large to store in userdata pointer.");
		body->SetUserData(reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t&>(ent)));
		return body;
	}

	void PhysicsSystem::destroyBody(b2Body* body) {
		ENGINE_DEBUG_ASSERT(body != nullptr, "Attempting to destroy null b2Body");
		physWorld.DestroyBody(body);
	}

	void PhysicsSystem::addListener(PhysicsListener* listener) {
		listeners.push_back(listener);
	}

	void PhysicsSystem::BeginContact(b2Contact* contact) {
		const auto entA = toEntity(contact->GetFixtureA()->GetBody()->GetUserData());
		const auto entB = toEntity(contact->GetFixtureB()->GetBody()->GetUserData());

		for (auto listener : listeners) {
			listener->beginContact(entA, entB);
		}
	}

	void PhysicsSystem::EndContact(b2Contact* contact) {
		const auto entA = toEntity(contact->GetFixtureA()->GetBody()->GetUserData());
		const auto entB = toEntity(contact->GetFixtureB()->GetBody()->GetUserData());

		for (auto listener : listeners) {
			listener->endContact(entA, entB);
		}
	}

	bool PhysicsSystem::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) {
		//   groupIndex = "I only collide with fixtures in this group" (int)
		// categoryBits = "I'm a ..." (bitset, usually just one bit should be set)
		//     maskBits = "I collide with categories ..." (bitset, all categories to collide with)
		const auto& filterA = fixtureA->GetFilterData();
		const auto& filterB = fixtureB->GetFilterData();

		// Are the fixtures in the same group and they both think they should collide?
		return (filterA.groupIndex == filterB.groupIndex)
			&& (filterA.maskBits & filterB.categoryBits)
			&& (filterB.maskBits & filterA.categoryBits);
	}
}
