// Game
#include <Game/World.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/systems/PhysicsInterpSystem.hpp>


namespace Game {
	void PhysicsInterpSystem::run(float32 dt) {
		const auto now = Engine::Clock::now();

		for (const auto& ent : world.getFilter<PhysicsInterpComponent, PhysicsProxyComponent>()) {
			const auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(ent);
			auto& physInterpComp = world.getComponent<PhysicsInterpComponent>(ent);

			Engine::Clock::TimePoint nextTime;
			Engine::Clock::TimePoint prevTime;
			Engine::Clock::TimePoint interpTime;
			const b2Transform* nextTrans = nullptr;
			const b2Transform* prevTrans = nullptr;

			if (physProxyComp.snap) {
				physInterpComp.trans = physProxyComp.trans;
				continue;
			} else {
				const auto tick = world.getTick();
				if (!world.hadComponent<PhysicsProxyComponent>(ent, tick)) {
					physInterpComp.trans = physProxyComp.trans;
					continue;
				}

				const auto& physProxyComp2 = world.getComponent<PhysicsProxyComponent>(ent, tick);
				prevTrans = &physProxyComp2.trans;
				prevTime = world.getTickTime(tick);

				nextTrans = &physProxyComp.trans;
				nextTime = world.getTickTime();

				interpTime = now - world.getTickInterval();
			}

			// TODO: use calcInterpValue and Math::lerp
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
