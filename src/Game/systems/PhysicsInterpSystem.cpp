// Game
#include <Game/World.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/systems/PhysicsInterpSystem.hpp>


namespace Game {
	void PhysicsInterpSystem::run(float32 dt) {
		const auto now = Engine::Clock::now();

		for (const auto& ent : world.getFilter<PhysicsBodyComponent, PhysicsInterpComponent>()) {
			const auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
			auto& physInterpComp = world.getComponent<PhysicsInterpComponent>(ent);

			Engine::Clock::TimePoint nextTime;
			Engine::Clock::TimePoint prevTime;
			Engine::Clock::TimePoint interpTime;
			const b2Transform* nextTrans = nullptr;
			const b2Transform* prevTrans = nullptr;

			if (physComp.snap) {
				physInterpComp.trans = physComp.getTransform();
				continue;
			} else {
				const auto tick = world.getTick();
				if (!world.hadComponent<PhysicsBodyComponent>(ent, tick)) {
					physInterpComp.trans = physComp.getTransform();
					continue;
				}

				const auto& physCompState2 = world.getComponentState<PhysicsBodyComponent>(ent, tick);
				prevTrans = &physCompState2.trans;
				prevTime = world.getTickTime(tick);

				nextTrans = &physComp.getTransform();
				nextTime = world.getTickTime();

				interpTime = now - world.getTickInterval();
			}

			// TODO: use calcInterpValue and Math::lerp
			const auto diff = nextTime - prevTime;
			auto a = (interpTime - prevTime).count() / static_cast<float64>(diff.count());
			a = std::min(1.0, std::max(a, 0.0));
			const auto b = 1 - a;

			{
				const float32 a2 = static_cast<float32>(a);
				const float32 b2 = static_cast<float32>(b);
				auto& lerpTrans = physInterpComp.trans;
				lerpTrans.p = a2 * nextTrans->p + b2 * prevTrans->p;

				// Normalized lerp - really should use slerp but since the delta is small nlerp is close to slerp
				lerpTrans.q.c = a2 * nextTrans->q.c + b2 * prevTrans->q.c;
				lerpTrans.q.s = a2 * nextTrans->q.s + b2 * prevTrans->q.s;
				const float32 mag = lerpTrans.q.c * lerpTrans.q.c + lerpTrans.q.s * lerpTrans.q.s;
				lerpTrans.q.c /= mag;
				lerpTrans.q.s /= mag;
			}
		}
	}
}
