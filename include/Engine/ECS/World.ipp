#pragma once

// Meta
#include <Meta/IndexOf.hpp>

// Engine
#include <Engine/ECS/World.hpp>


namespace Engine::ECS {
	WORLD_TPARAMS
	template<class Arg>
	WORLD_CLASS::World(Arg& arg)
		: systems((sizeof(Ss*), arg) ...)
		, beginTime{Clock::now()} {
		activeSnap.tickTime = beginTime;
		(getSystem<Ss>().setup(), ...);
	}

	WORLD_TPARAMS
	void WORLD_CLASS::run() {
		const auto endTime = Clock::now();
		deltaTimeNS = endTime - beginTime;
		beginTime = endTime;
		deltaTime = Clock::Seconds{deltaTimeNS}.count();

		//if (beginTime - activeSnap.tickTime > maxDelay) {
		//	ENGINE_WARN("World tick falling behind by ",
		//		Clock::Seconds{beginTime - activeSnap.tickTime - maxDelay}.count(), "s"
		//	);
		//
		//	// We could instead limit the number of ticks in the while loop
		//	// which would have the effect of slowing down the world instead of
		//	// throwing away time like this does
		//	activeSnap.tickTime = beginTime - maxDelay;
		//}

		if constexpr (ENGINE_CLIENT) {
			if (rollbackData.tick != -1 && !performingRollback) {
				const auto oldTick = activeSnap.currTick;
				const auto oldTime = activeSnap.tickTime;

				const auto* snap = snapBuffer.find(rollbackData.tick);
				if (!snap) {
					ENGINE_WARN("Unable to perform world rollback to tick ", rollbackData.tick);
					rollbackData.tick = -1;
					return;
				}

				rollbackData.tick = activeSnap.currTick;
				rollbackData.time = activeSnap.tickTime;
				performingRollback = true;
				ENGINE_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ", activeSnap.currTick);
				loadSnapshot(*snap);
			}

			/*if (activeSnap.currTick > 64*10 && activeSnap.currTick % 128 == 0 && last != activeSnap.currTick) {
				last = activeSnap.currTick;
				ENGINE_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ", activeSnap.currTick);
				const auto oldTick = activeSnap.currTick;
				const auto oldTime = activeSnap.tickTime;

				const auto& snap = snapBuffer.get(activeSnap.currTick - SnapshotCount + 1);
				loadSnapshot(snap);

				const auto startTime = Clock::now();
				performingRollback = true;
				while (activeSnap.currTick < oldTick) {
					const auto& found = snapBuffer.get(activeSnap.currTick + 1);
					const auto nextTickTime = found.tickTime;
					tickSystems();
					activeSnap.tickTime = nextTickTime;
				}
				performingRollback = false;
				activeSnap.tickTime = oldTime;

				const auto diff = Clock::Milliseconds{Clock::now() - startTime}.count();
				ENGINE_LOG("Rollback took: ", (diff < (1000.0 / TickRate) ? Engine::ASCII_SUCCESS : Engine::ASCII_ERROR), diff, "ms");
				ENGINE_DEBUG_ASSERT(oldTick == activeSnap.currTick);
				ENGINE_DEBUG_ASSERT(oldTime == activeSnap.tickTime);

				ENGINE_LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ", activeSnap.currTick);
			}*/
		}

		if (ENGINE_CLIENT && performingRollback) {
			while (activeSnap.currTick < rollbackData.tick) {
				const auto& found = snapBuffer.get(activeSnap.currTick + 1);
				const auto nextTickTime = found.tickTime;
				tickSystems();
				ENGINE_LOG("Rollback: ", activeSnap.currTick);
				activeSnap.tickTime = nextTickTime;
			}

			if (activeSnap.currTick == rollbackData.tick) {
				activeSnap.tickTime = rollbackData.time;
				performingRollback = false;
				rollbackData.tick = -1;
				ENGINE_LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ", activeSnap.currTick);
				__debugbreak();
			}
		} else {
			while (activeSnap.tickTime + tickInterval <= beginTime) {
				tickSystems();
				activeSnap.tickTime += std::chrono::duration_cast<Clock::Duration>(tickInterval * tickScale);
			}
		}
		
		(getSystem<Ss>().run(deltaTime), ...);
	}

	WORLD_TPARAMS
	void WORLD_CLASS::tickSystems() {
		++activeSnap.currTick;

		storeSnapshot();
		(getSystem<Ss>().preTick(), ...);


		//auto&& tickSystem = [&]<class S>(){
		//	if (performingRollback) {
		//		ENGINE_LOG("Running system: ", typeid(S).name());
		//		//__debugbreak();
		//	}
		//	getSystem<S>().tick();
		//}; (tickSystem.operator()<Ss>(), ...);

		(getSystem<Ss>().tick(), ...);
		(getSystem<Ss>().postTick(), ...);
	}

	WORLD_TPARAMS
	template<class System>
	constexpr static SystemId WORLD_CLASS::getSystemId() noexcept {
		return Meta::IndexOf<System, Ss...>::value;
	}

	WORLD_TPARAMS
	template<class System>
	System& WORLD_CLASS::getSystem() {
		return std::get<System>(systems);
	}

	WORLD_TPARAMS
	template<class... SystemN>
	SystemBitset WORLD_CLASS::getBitsetForSystems() const {
		SystemBitset value;
		((value[getSystemId<SystemN>()] = true), ...);
		return value;
	};

	WORLD_TPARAMS
	float32 WORLD_CLASS::getTickRatio() const {
		// TODO: is this correct? dont think so
		return (activeSnap.tickTime - beginTime).count() / static_cast<float32>(tickInterval.count());
	}

	WORLD_TPARAMS
	float32 WORLD_CLASS::getDeltaTime() const {
		return deltaTime;
	}

	WORLD_TPARAMS
	auto WORLD_CLASS::getDeltaTimeNS() const {
		return deltaTimeNS;
	}

	WORLD_TPARAMS
	template<class SystemA, class SystemB>
	constexpr static bool WORLD_CLASS::orderBefore() {
		return Meta::IndexOf<SystemA, Ss...>::value < Meta::IndexOf<SystemB, Ss...>::value;
	}

	WORLD_TPARAMS
	template<class SystemA, class SystemB>
	constexpr static bool WORLD_CLASS::orderAfter() {
		return orderBefore<SystemB, SystemA>();
	}
	
	WORLD_TPARAMS
	template<class Callable>
	void WORLD_CLASS::callWithComponent(ComponentId cid, Callable&& callable) {
		using Caller = void(Callable::*)(void) const;
		constexpr Caller callers[]{ &Callable::operator()<Cs>... };
		return (callable.*callers[cid])();
	}

	WORLD_TPARAMS
	void WORLD_CLASS::storeSnapshot() {
		// TODO: delta compression?

		(getSystem<Ss>().preStoreSnapshot(), ...);
		const auto oldSeq = tick() - snapBuffer.capacity();
		const auto* found = snapBuffer.find(oldSeq);
		if (found) {
			activeSnap.destroyEntities(found->markedForDeath);
			//snapBuffer.remove(oldSeq); // TODO: should be rm when we insert over it? yes?
		}

		auto& snap = snapBuffer.insert(tick());
		snap.assign(activeSnap);
	}

	WORLD_TPARAMS
	void WORLD_CLASS::loadSnapshot(const RollbackSnapshot& snap) {
		activeSnap.assign(snap);
		(getSystem<Ss>().postLoadSnapshot(), ...);
		--activeSnap.currTick; // We need to do this because tickSystems increments tick.
	}
}
