// Game
#include <Game/World.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/systems/PhysicsInterpSystem.hpp>


namespace Game {
	void PhysicsInterpSystem::run(float32 dt) {
		const auto now = Engine::Clock::now();
		/*float32 a = timeDiff.count() / static_cast<float32>(interval.count());
		//float32 a = (Engine::Clock::now() - world.getTickTime()).count() / static_cast<float32>(world.getTickInterval().count());

		// TODO: how are we getting negative numbers????
		if (a < 0.0f || a > 1.0f) {
			//ENGINE_WARN("This should never happen: ",
			//	"(", now.time_since_epoch().count(), " - ", tickTime.time_since_epoch().count(), ") / ", interval.count(), " = ", a,
			//	"\n\n\n\n");
		}

		////////////////////////
		////////////////////////
		////////////////////////
		////////////////////////
		// TODO: this will be wrong for multi frame interp for remote entities
		////////////////////////
		////////////////////////
		////////////////////////
		////////////////////////
		////////////////////////
		////////////////////////
		a = std::min(1.0f, std::max(0.0f, a));
		const float32 b = 1.0f - a;*/

		int buffSize = 0;
		Engine::Clock::Duration ping = {};
		for (const auto& ply : world.getFilter<ActionComponent, ConnectionComponent>()) {
			const auto& connComp = world.getComponent<ConnectionComponent>(ply);
			const auto& actComp = world.getComponent<ActionComponent>(ply);
			buffSize = static_cast<int>(actComp.estBufferSize) + 1;
			ping = connComp.conn->getPing() + connComp.conn->getJitter();
			break;
		}

		for (const auto& ent : world.getFilter<PhysicsInterpComponent, PhysicsBodyComponent>()) {
			const auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(ent);
			auto& physInterpComp = world.getComponent<PhysicsInterpComponent>(ent);

			Engine::Clock::TimePoint nextTime;
			Engine::Clock::TimePoint prevTime;
			auto interpTime = now;
			const b2Transform* nextTrans = nullptr;
			const b2Transform* prevTrans = nullptr;
			if (physProxyComp.snap) {
				physInterpComp.trans = physProxyComp.trans;
				continue;
			} else if (physInterpComp.onlyUserVerified) {
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
				interpTime = now - step;
				
				// TODO: this isnt great on the ECS/snapshot memory layout
				for (Engine::ECS::Tick t = world.getTick() - 1; world.hasHistory(t); --t) {
					// TODO: idealy this would have hasComponent(ent, tick) check

					const auto& physProxyComp2 = world.getComponent<PhysicsProxyComponent>(ent, t);
					if (physProxyComp2.rollbackOverride) {
						const auto tickTime = world.getTickTime();
						if (tickTime >= interpTime) {
							nextTrans = &physProxyComp2.trans;
							nextTime = tickTime;
						} else {
							prevTrans = &physProxyComp2.trans;
							prevTime = tickTime;
							break;
						}
					}
				}

				if (!nextTrans) {
					ENGINE_WARN("nextTrans not found!");
					if (prevTrans) {
						physInterpComp.trans = *prevTrans;
					}
					continue;
				}

				if (!prevTrans) {
					ENGINE_WARN("prevTrans not found!");
					if (nextTrans) {
						physInterpComp.trans = *nextTrans;
					}
					continue;
				}
			} else {
				prevTrans = &physProxyComp.trans;

				nextTrans = &physProxyComp.trans;
				prevTime = world.getTickTime();
				nextTime = prevTime + world.getTickInterval();
			}

			const auto diff = nextTime - prevTime;
			auto a = (interpTime - prevTime).count() / static_cast<float64>(diff.count());
			a = std::min(1.0, std::max(a, 0.0));
			const auto b = 1 - a;

			float32 a2 = static_cast<float32>(a);
			float32 b2 = static_cast<float32>(b);
			{
				float32 a = a2;
				float32 b = b2;
				auto& lerpTrans = physInterpComp.trans;
				lerpTrans.p = a * nextTrans->p + b * prevTrans->p;

				// Normalized lerp - really should use slerp but since the delta is small nlerp is close to slerp
				lerpTrans.q.c = a * nextTrans->q.c + b * prevTrans->q.c;
				lerpTrans.q.s = a * nextTrans->q.s + b * prevTrans->q.s;
				const float32 mag = lerpTrans.q.c * lerpTrans.q.c + lerpTrans.q.s * lerpTrans.q.s;
				lerpTrans.q.c /= mag;
				lerpTrans.q.s /= mag;
			}
		}
	}
}
