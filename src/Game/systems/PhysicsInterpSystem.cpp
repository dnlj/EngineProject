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

		for (const auto& ent : world.getFilter<PhysicsInterpComponent, PhysicsComponent>()) {
			const auto& physComp = world.getComponent<PhysicsComponent>(ent);
			auto& physInterpComp = world.getComponent<PhysicsInterpComponent>(ent);
			
			Engine::Clock::Duration step = {};
			Engine::ECS::Tick nextTick = 0; // TODO: rm - unused
			Engine::ECS::Tick prevTick = 0; // TODO: rm - unused
			Engine::Clock::TimePoint nextTime;
			Engine::Clock::TimePoint prevTime;
			auto interpTime = now;
			const b2Transform* nextTrans = nullptr;
			const b2Transform* prevTrans = nullptr;
			if (physComp.snap) {
				physInterpComp.trans = physComp.getTransform();
				continue;
			} else if (physInterpComp.onlyUserVerified) {
				step = Engine::Clock::Duration{std::chrono::milliseconds{200}}; // TODO: dont hard code. figure out a good number. 
				//step += 3*world.getTickInterval();

				interpTime = now - step;

				// TODO: check active comp
				//if (physComp.rollbackOverride) {
				//	nextTrans = &physComp.getStored().trans;
				//	nextTime = world.getTickTime();
				//}

				// TODO: this isnt great on the ECS/snapshot memory layout
				for (Engine::ECS::Tick t = world.getTick() - 1; t > world.getTick() - world.getSnapshotCount(); --t) {
					const auto* snap = world.getSnapshot(t);
					if (!snap || !snap->hasComponent<PhysicsComponent>(ent)) { break; }

					const auto& pc = snap->getComponent<PhysicsComponent>(ent);
					if (pc.rollbackOverride) {
						if (snap->tickTime >= interpTime) {
							nextTrans = &pc.getStored().trans;
							nextTime = snap->tickTime;
							nextTick = t;
						} else {
							prevTrans = &pc.getStored().trans;
							prevTime = snap->tickTime;
							prevTick = t;
							break;
						}
					}
				}
			} else {
				prevTrans = &physComp.getStored().trans;

				nextTrans = &physComp.getTransform();
				prevTime = world.getTickTime();
				nextTime = prevTime + world.getTickInterval();
			}

			if (!nextTrans) { continue; }
			if (!prevTrans) { prevTrans = nextTrans; }


			// TODO: would the variation in diff be the reason for the stutter?
			const auto diff = nextTime - prevTime;
			float32 a = (interpTime - prevTime).count() / static_cast<float32>(diff.count());
			if (a < 0.0f || a > 1.0f) { ENGINE_WARN("This should never happen: ", a); }
			a = std::min(1.0f, std::max(a, 0.0f));
			const float32 b = 1 - a;

			if (step != decltype(step){}) {
				ENGINE_LOG("Interp: ", a, " ", b,
					"\n\t       step: ", step.count(),
					"\n\t   prevTime: ", prevTime.time_since_epoch().count(), " - ", prevTick,
					"\n\t   nextTime: ", nextTime.time_since_epoch().count(), " - ", nextTick,
					"\n\t interpTime: ", interpTime.time_since_epoch().count(),
					"\n\t        now: ", now.time_since_epoch().count(), " - ", world.getTick(),
					"\n\t       diff: ", diff.count()
				);
			}

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
