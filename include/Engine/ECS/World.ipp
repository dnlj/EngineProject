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

		// TODO: shouldnt this be after rollback code?
		if (beginTime - activeSnap.tickTime > maxDelay) {
			ENGINE_WARN("World tick falling behind by ",
				Clock::Seconds{beginTime - activeSnap.tickTime - maxDelay}.count(), "s"
			);
		
			// We could instead limit the number of ticks in the while loop
			// which would have the effect of slowing down the world instead of
			// throwing away time like this does
			activeSnap.tickTime = beginTime - maxDelay;
		}

		if constexpr (ENGINE_CLIENT) {
			if (activeSnap.currTick > 64*10 && activeSnap.currTick % 128 == 0 && !performingRollback) {
				ENGINE_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ", activeSnap.currTick);
				const auto oldTick = activeSnap.currTick;
				const auto oldTime = activeSnap.tickTime;

				const auto& snap = snapBuffer.get(activeSnap.currTick - SnapshotCount);
				loadSnapshot(snap);
				
				performingRollback = true;
				while (activeSnap.currTick < oldTick) {
					tickSystems();
					// TODO: need to increment tick time. use values from snapshotbuffer
				}

				activeSnap.tickTime = oldTime;
				
				ENGINE_ASSERT(oldTick == activeSnap.currTick);
				ENGINE_ASSERT(oldTime == activeSnap.tickTime);

				ENGINE_LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ", activeSnap.currTick);
			}
		}

		while (activeSnap.tickTime + tickInterval <= beginTime) {
			performingRollback = false; // TODO this reall should be right after the rollback loop but it is here temp
			tickSystems();
			activeSnap.tickTime += std::chrono::duration_cast<Clock::Duration>(tickInterval * tickScale);
		}

		(getSystem<Ss>().run(deltaTime), ...);
	}

	WORLD_TPARAMS
	void WORLD_CLASS::tickSystems() {
		//ENGINE_INFO("Tick: ", getTick());
		// TODO: do we actually use tickDeltaTime in any systems? maybe just make an accessor/var on world
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
		++activeSnap.currTick;
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
	}
}
