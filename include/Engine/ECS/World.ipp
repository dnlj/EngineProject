// Meta
#include <Meta/IndexOf.hpp>

// Engine
#include <Engine/ECS/World.hpp>


namespace Engine::ECS {
	ECS_WORLD_TPARAMS
	ECS_WORLD_CLASS::Snapshot::Snapshot()
		: compContainers{(IsSnapshotRelevant<Cs> ? (new SnapshotTraits<Cs>::Container()) : nullptr)... } {
	}

	ECS_WORLD_TPARAMS
	ECS_WORLD_CLASS::Snapshot::~Snapshot() {
		((delete getComponentContainer_Unsafe<Cs>()), ...);
	}
}

namespace Engine::ECS {
	ECS_WORLD_TPARAMS
	template<class Arg>
	ECS_WORLD_CLASS::World(Arg&& arg)
		: beginTime{Clock::now()}
		// TODO: add static_assert with requires statement to check this and give nice error message. Currently requires is not correctly supported on MSVC. See https://developercommunity.visualstudio.com/t/Keyword-requires-within-if-statement-d/1287202
		// 
		// If you are here from a compile error make sure your system has the correct constructor. Usually `using System::System` will work fine.
		// You might be seeing this because you just added a member to an otherwise empty system.
		//
		, compContainers{ new ComponentContainer<Cs>()... }
		, systems{ new Ss(std::forward<Arg>(arg))... } {

		tickTime = beginTime;
		(getSystem<Ss>().setup(), ...);
	}

	ECS_WORLD_TPARAMS
	ECS_WORLD_CLASS::~World() {
		((delete &getSystem<Ss>()), ...);
		((delete &getComponentContainer<Cs>()), ...);
	}

	ECS_WORLD_TPARAMS
	void ECS_WORLD_CLASS::run() {
		const auto endTime = Clock::now();
		deltaTimeNS = endTime - beginTime;
		beginTime = endTime;
		deltaTime = Clock::Seconds{deltaTimeNS}.count();
		deltaTimeSmooth = dtSmoothing * deltaTime + (1 - dtSmoothing) * deltaTimeSmooth;

		if constexpr (ENGINE_CLIENT) {
			if (rollbackData.tick != -1 && !performingRollback) {
				const auto oldTick = currTick;
				const auto oldTime = tickTime;

				if (loadSnapshot(rollbackData.tick)) {
					ENGINE_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ", rollbackData.tick, " ", currTick);
					rollbackData.tick = oldTick;
					rollbackData.time = oldTime;
					performingRollback = true;
				} else {
					ENGINE_WARN("Unable to perform world rollback to tick ", rollbackData.tick, " ", oldTick);
					rollbackData.tick = -1;
				}
			}
		} else {
			ENGINE_DEBUG_ASSERT(performingRollback == false, "Rollback is currently only used client side");
		}

		if (ENGINE_CLIENT && performingRollback) {
			while (currTick < rollbackData.tick) {
				const auto& found = history.get(currTick + 1);
				const auto nextTickTime = found.tickTime;
				tickSystems();
				ENGINE_LOG("Rollback2: ", currTick);
				tickTime = nextTickTime;
			}

			if (currTick == rollbackData.tick) {
				tickTime = rollbackData.time;
				performingRollback = false;
				rollbackData.tick = -1;
				ENGINE_LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ", currTick);
			}
		} else {
			constexpr auto maxTickCount = 6;
			int tickCount = -1;
			while (tickTime + tickInterval <= beginTime && ++tickCount < maxTickCount) {
				tickSystems();
				tickTime += std::chrono::duration_cast<Clock::Duration>(tickInterval * tickScale);
			}
		}

		updateSystems();
		destroyMarkedEntities();
	}

	ECS_WORLD_TPARAMS
	void ECS_WORLD_CLASS::tickSystems() {
		++currTick;

		// Currently rollback can only happen client side so there is no point in storing snapshots server side.
		if (ENGINE_CLIENT) { storeSnapshot(); }

		(getSystem<Ss>().preTick(), ...);
		(getSystem<Ss>().tick(), ...);
		(getSystem<Ss>().postTick(), ...);
	}

	ECS_WORLD_TPARAMS
	void ECS_WORLD_CLASS::updateSystems() {
		++currUpdate;
		(getSystem<Ss>().update(deltaTime), ...);
	}

	ECS_WORLD_TPARAMS
	void ECS_WORLD_CLASS::storeSnapshot() {
		(getSystem<Ss>().preStoreSnapshot(), ...);

		auto& snap = history.insertNoInit(currTick);
		snap.tickTime = tickTime;
		Meta::ForEach<Cs...>::call([&]<class C>{
			if constexpr (IsSnapshotRelevant<C>) {
				auto& cont = getComponentContainer<C>();
				auto& scont = snap.getComponentContainer<C>();
				scont.clear();

				for (const auto& [ent, comp] : cont) {
					ENGINE_FLATTEN std::apply(
						[&]<class... Args>(Args&&... args) ENGINE_INLINE {
							scont.add(ent, std::forward<Args>(args)...);
						},
						SnapshotTraits<C>::toSnapshot(comp)
					);
				}
			}
		});
	}

	ECS_WORLD_TPARAMS
	bool ECS_WORLD_CLASS::loadSnapshot(Tick tick) {
		if (!history.contains(tick)) { return false; }

		auto& snap = history.get(tick);
		Meta::ForEach<Cs...>::call([&]<class C>{
			if constexpr (IsSnapshotRelevant<C>) {
				auto& cont = getComponentContainer<C>();
				auto& scont = snap.getComponentContainer<C>();
				for (auto& [ent, comp] : scont) {
					if (cont.contains(ent)) {
						SnapshotTraits<C>::fromSnapshot(cont.get(ent), comp);
					}
				}
			}
		});

		currTick = tick - 1;
		tickTime = snap.tickTime;

		(getSystem<Ss>().postLoadSnapshot(), ...);
		return true;
	}
}

template ECS_WORLD_TYPE::World(ECS_WORLD_ARG);
template void ECS_WORLD_TYPE::run();
template ECS_WORLD_TYPE::~World();
