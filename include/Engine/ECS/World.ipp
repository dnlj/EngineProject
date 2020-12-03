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
		, beginTime{Clock::now()}
		, tickTime{beginTime} {
		(getSystem<Ss>().setup(), ...);
	}

	WORLD_TPARAMS
	void WORLD_CLASS::run() {
		const auto endTime = Clock::now();
		deltaTimeNS = endTime - beginTime;
		beginTime = endTime;
		deltaTime = Clock::Seconds{deltaTimeNS}.count();

		constexpr auto maxTickCount = 6;
		int tickCount = -1;
		while (tickTime + tickInterval <= beginTime && ++tickCount < maxTickCount) {
			tickSystems();
			tickTime += std::chrono::duration_cast<Clock::Duration>(tickInterval * tickScale);
		}
		
		(getSystem<Ss>().run(deltaTime), ...);

		// TODO: rollback update: delete markedForDeath entities
	}

	WORLD_TPARAMS
	void WORLD_CLASS::tickSystems() {
		++currTick;

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
	void WORLD_CLASS::setNextTick(Tick tick) {
		// TODO: defer this till next `run`
		currTick = tick - 1;
		tickTime = Clock::now();
	}
}
