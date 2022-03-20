// Meta
#include <Meta/IndexOf.hpp>

// Engine
#include <Engine/ECS/World.hpp>


namespace Engine::ECS {
	WORLD_TPARAMS
	WORLD_CLASS::Snapshot::Snapshot()
		: compContainers{(IsSnapshotRelevant<Cs>::value ? (new ComponentContainerForSnapshot<Cs>::Type()) : nullptr)... } {
	}

	WORLD_TPARAMS
	WORLD_CLASS::Snapshot::~Snapshot() {
		((delete getComponentContainer_Unsafe<Cs>()), ...);
	}
}

namespace Engine::ECS {
	WORLD_TPARAMS
	template<class Arg>
	WORLD_CLASS::World(Arg&& arg)
		: beginTime{Clock::now()}
		// TODO: add static_assert with requires statement to check this and give nice error message. Currently requires is not correctly supported on MSVC. See https://developercommunity.visualstudio.com/t/Keyword-requires-within-if-statement-d/1287202
		// 
		// If you are here from a compile error make sure your system has the correct constructor. Usually `using System::System` will work fine.
		// You might be seeing this because you just added a member to an otherwise empty system.
		//
		, systems{ new Ss(std::forward<Arg>(arg))... }
		, compContainers{ new ComponentContainer<Cs>()... } {

		tickTime = beginTime;
		(getSystem<Ss>().setup(), ...);
	}

	WORLD_TPARAMS
	WORLD_CLASS::~World() {
		((delete &getSystem<Ss>()), ...);
		((delete &getComponentContainer<Cs>()), ...);
	}

	WORLD_TPARAMS
	void WORLD_CLASS::run() {
		const auto endTime = Clock::now();
		deltaTimeNS = endTime - beginTime;
		beginTime = endTime;
		deltaTime = Clock::Seconds{deltaTimeNS}.count();

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

		(getSystem<Ss>().run(deltaTime), ...);
		destroyMarkedEntities();
	}

	WORLD_TPARAMS
	void WORLD_CLASS::tickSystems() {
		++currTick;

		storeSnapshot();
		(getSystem<Ss>().preTick(), ...);
		(getSystem<Ss>().tick(), ...);
		(getSystem<Ss>().postTick(), ...);
	}

	WORLD_TPARAMS
	void WORLD_CLASS::storeSnapshot() {
		(getSystem<Ss>().preStoreSnapshot(), ...);

		auto& snap = history.insertNoInit(currTick);
		snap.tickTime = tickTime;
		Meta::ForEach<Cs...>::call([&]<class C>{
			if constexpr (IsSnapshotRelevant<C>::value) {
				auto& cont = getComponentContainer<C>();
				auto& scont = snap.getComponentContainer<C>();
				scont.clear();
				for (const auto& [ent, comp] : cont) {
					scont.add(ent, comp);
				}
			}
		});
	}

	WORLD_TPARAMS
	bool WORLD_CLASS::loadSnapshot(Tick tick) {
		if (!history.contains(tick)) { return false; }

		auto& snap = history.get(tick);
		Meta::ForEach<Cs...>::call([&]<class C>{
			if constexpr (IsSnapshotRelevant<C>::value) {
				auto& cont = getComponentContainer<C>();
				auto& scont = snap.getComponentContainer<C>();
				for (auto& [ent, comp] : scont) {
					if (cont.contains(ent)) {
						cont.get(ent) = comp;
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
