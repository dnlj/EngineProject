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

		/*
		if constexpr (ENGINE_CLIENT) {
			if (activeSnap.currTick > 64*10 && activeSnap.currTick % 128 == 0 && !performingRollback) {
				ENGINE_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ", activeSnap.currTick);
				const auto oldTick = activeSnap.currTick;
				const auto oldTime = activeSnap.tickTime;

				const auto& snap = snapBuffer.get(currTick - SnapshotCount);
				loadSnapshot(snap);

				performingRollback = true;
				while (activeSnap.currTick < oldTick) {
					tickSystems();
					// TODO: need to increment tick time. use values from snapshotbuffer
				}

				activeSnap.tickTime = oldTime;

				ENGINE_ASSERT(oldTick == currTick);
				ENGINE_ASSERT(oldTime == tickTime);

				ENGINE_LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ", activeSnap.currTick);
			}
		}*/

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
		//storeSnapshot();
		(getSystem<Ss>().preTick(), ...);
		(getSystem<Ss>().tick(), ...);
		(getSystem<Ss>().postTick(), ...);
		activeSnap.markedForDeath.clear();
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
	void WORLD_CLASS::destroyEntity(Entity ent) {
		ENGINE_WARN("DESTROY ENTITY: ", ent);
		removeAllComponents(ent);
		
		#if defined(DEBUG)
			if (!isAlive(ent)) {
				ENGINE_ERROR("Attempting to destroy an already dead entity \"", ent, "\"");
			} else if (entities[ent.id].ent.gen != ent.gen) {
				ENGINE_ERROR(
					"Attempting to destroy an old generation entity. Current generation is ",
					entities[ent.id].ent.gen,
					" attempted to delete ",
					ent.gen
				);
			}
		#endif

		deadEntities.push_back(ent);
		entities[ent.id].state = EntityState::Dead;

		// TODO: shouldnt this be called before the entity is actually destroyed? bitset will be empty at this time.
		fm.onEntityDestroyed(ent, compBitsets[ent.id]);
	}

	WORLD_TPARAMS
	template<class... Components>
	auto WORLD_CLASS::getFilterFor() -> Filter& {
		return fm.getFilterFor(self(), activeSnap.getBitsetForComponents<Components...>());
	}

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

	/*
	WORLD_TPARAMS
	void WORLD_CLASS::storeSnapshot() {
		// TODO: delta compression?
		// TODO: Make sure we are storing all tick time info at the correct time: currentTick, beginTime, tickTime, etc.

		(getSystem<Ss>().preStoreSnapshot(), ...);

		auto& snap = snapBuffer.insert(currTick);

		for (const auto ent : snap.markedForDeath) {
			destroyEntity(ent);
		}

		snap.tickTime = tickTime;
		snap.currTick = currTick;
		snap.entities = entities;
		snap.deadEntities = deadEntities;
		snap.markedForDeath = markedForDeath;
		snap.compBitsets = compBitsets;

		const auto& storeComps = [&]<class C>(){
			if constexpr (IsSnapshotRelevant<C>::value) {
				constexpr auto cid = getComponentId<C>();
				std::get<cid>(snap.compContainers) = std::get<cid>(compContainers);
			}
		};
		
		(storeComps.operator()<Cs>(), ...);
	}

	WORLD_TPARAMS
	void WORLD_CLASS::loadSnapshot(const RollbackSnapshot& snap) {
		// TODO: This breaks filters. Need to rework them or also rollback them

		tickTime = snap.tickTime;
		currTick = snap.currTick;
		entities = snap.entities;
		deadEntities = snap.deadEntities;
		markedForDeath = snap.markedForDeath;
		compBitsets = snap.compBitsets;

		const auto& loadComps = [&]<class C>(){
			if constexpr (IsSnapshotRelevant<C>::value) {
				constexpr auto cid = getComponentId<C>();
				std::get<cid>(compContainers) = std::get<cid>(snap.compContainers);
			}
		};
		
		(loadComps.operator()<Cs>(), ...);
		(getSystem<Ss>().postLoadSnapshot(), ...);
	}*/
}
