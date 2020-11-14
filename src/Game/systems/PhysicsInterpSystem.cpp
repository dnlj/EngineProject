// Game
#include <Game/World.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/systems/PhysicsInterpSystem.hpp>


namespace Game {
	void PhysicsInterpSystem::run(float32 dt) {
		const auto interval = world.getTickInterval();
		const auto now = Engine::Clock::now();
		const auto tickTime = world.getTickTime();
		const auto timeDiff = now - tickTime;
		float32 a = timeDiff.count() / static_cast<float32>(interval.count());
		//float32 a = (Engine::Clock::now() - world.getTickTime()).count() / static_cast<float32>(world.getTickInterval().count());

		// TODO: how are we getting negative numbers????
		if (a < 0.0f || a > 1.0f) {
			//ENGINE_WARN("This should never happen: ",
			//	"(", now.time_since_epoch().count(), " - ", tickTime.time_since_epoch().count(), ") / ", interval.count(), " = ", a,
			//	"\n\n\n\n");
		}

		a = std::min(1.0f, std::max(0.0f, a));
		const float32 b = 1.0f - a;

		for (const auto& ent : world.getFilter<PhysicsInterpComponent, PhysicsComponent>()) {
			const auto& physComp = world.getComponent<PhysicsComponent>(ent);
			auto& physInterpComp = world.getComponent<PhysicsInterpComponent>(ent);
			auto& lerpTrans = physInterpComp.trans;

			const auto* lastSnap = world.getSnapshot(world.getTick() - 1);

			if (physComp.snap || !lastSnap || !lastSnap->isValid(ent) || !lastSnap->hasComponent<PhysicsComponent>(ent)) {
				lerpTrans = physComp.getTransform();
			} else {
				const auto& lastPhysComp = lastSnap->getComponent<PhysicsComponent>(ent);

				// TODO: shouldnt this be current stored? why last? stored > update > current
				//const auto& prevTrans = lastPhysComp.getStored().trans;
				const auto& prevTrans = physComp.getStored().trans;

				//const auto& nextTrans = physComp.getStored().trans;
				const auto& nextTrans = physComp.getTransform(); // TODO: this is more responsive but glitches on server update.
				
				lerpTrans.p = a * nextTrans.p + b * prevTrans.p;

				// Normalized lerp - really should use slerp but since the delta is small nlerp is close to slerp
				lerpTrans.q.c = a * nextTrans.q.c + b * prevTrans.q.c;
				lerpTrans.q.s = a * nextTrans.q.s + b * prevTrans.q.s;
				const float32 mag = lerpTrans.q.c * lerpTrans.q.c + lerpTrans.q.s * lerpTrans.q.s;
				lerpTrans.q.c /= mag;
				lerpTrans.q.s /= mag;
			}
		}
	}
}
