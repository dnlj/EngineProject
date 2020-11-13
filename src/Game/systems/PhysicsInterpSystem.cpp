// Game
#include <Game/World.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/systems/PhysicsInterpSystem.hpp>


namespace Game {
	void PhysicsInterpSystem::run(float32 dt) {
		const float32 a = world.getTickRatio();
		const float32 b = 1.0f - a;

		for (const auto& ent : world.getFilter<PhysicsInterpComponent, PhysicsComponent>()) {
			const auto& physComp = world.getComponent<PhysicsComponent>(ent);
			auto& physInterpComp = world.getComponent<PhysicsInterpComponent>(ent);

			const auto* lastSnap = world.getSnapshot(world.getTick() - 1);
			if (!lastSnap || !lastSnap->isValid(ent) || !lastSnap->hasComponent<PhysicsComponent>(ent)) {
				physInterpComp.trans = physComp.getTransform();
			} else {
				const auto& lastPhysComp = lastSnap->getComponent<PhysicsComponent>(ent);
				physInterpComp.trans.p = a * physComp.getTransform().p + b * lastPhysComp.getTransform().p;
			}

			// TODO: rotation
		}

		/* old code

		for (auto ent : world.getFilter<Filter>()) {
			auto& physComp = world.getComponent<PhysicsComponent>(ent);
			const auto& prevTrans = physComp.prevTransform;
			const auto& nextTrans = physComp.getBody().GetTransform();
			auto& lerpTrans = physComp.interpTransform;

			lerpTrans.p = a * nextTrans.p + b * prevTrans.p;

			// Normalized lerp - really should use slerp but since the delta is small nlerp is close to slerp
			lerpTrans.q.c = a * nextTrans.q.c + b * prevTrans.q.c;
			lerpTrans.q.s = a * nextTrans.q.s + b * prevTrans.q.s;
			const float32 mag = lerpTrans.q.c * lerpTrans.q.c + lerpTrans.q.s * lerpTrans.q.s;
			lerpTrans.q.c /= mag;
			lerpTrans.q.s /= mag;


			lerpTrans = nextTrans; // TODO: rm
		}

		*/
	}
}
