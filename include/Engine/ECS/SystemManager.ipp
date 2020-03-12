#pragma once

// Engine
#include <Engine/Engine.hpp>

// Meta
#include <Meta/IndexOf.hpp>


namespace Engine::ECS {
	template<template<class...> class SystemsType, class... Systems>
	template<class Arg>
	SystemManager<SystemsType<Systems...>>::SystemManager(float tickInterval, Arg& arg)
		// Using sizeof here allows us to use the comma operator to duplicate `arg` N times
		: tickInterval{tickInterval}
		, systems((sizeof(Systems*), arg) ...) {

		(getSystem<Systems>().setup(), ...);
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System>
	constexpr static SystemID SystemManager<SystemsType<Systems...>>::getSystemID() noexcept {
		return Meta::IndexOf<System, Systems...>::value;
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System>
	System& SystemManager<SystemsType<Systems...>>::getSystem() {
		return std::get<System>(systems);
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System1, class System2, class... SystemN>
	SystemBitset SystemManager<SystemsType<Systems...>>::getBitsetForSystems() const {
		return getBitsetForSystems<System1>() |= (getBitsetForSystems<System2>() |= ... |= getBitsetForSystems<SystemN>());
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System1>
	SystemBitset SystemManager<SystemsType<Systems...>>::getBitsetForSystems() const {
		const auto sid = getSystemID<System1>();
		SystemBitset value;
		value[sid] = true;
		return value;
	}

	template<template<class...> class SystemsType, class... Systems>
	SystemManager<SystemsType<Systems...>>::~SystemManager() {
	}

	template<template<class...> class SystemsType, class... Systems>
	void SystemManager<SystemsType<Systems...>>::run(float dt) {
		tickAccum += dt;
		while (tickInterval < tickAccum) {
			(getSystem<Systems>().tick(tickInterval), ...);
			tickAccum -= tickInterval;
		}

		(getSystem<Systems>().run(dt), ...);
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class SystemA, class SystemB>
	constexpr static bool SystemManager<SystemsType<Systems...>>::orderBefore() {
		return Meta::IndexOf<SystemA, Systems...>::value < Meta::IndexOf<SystemB, Systems...>::value;
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class SystemA, class SystemB>
	constexpr static bool SystemManager<SystemsType<Systems...>>::orderAfter() {
		return SystemManager::orderBefore<SystemB, SystemA>();
	}
	
	template<template<class...> class SystemsType, class... Systems>
	float32 SystemManager<SystemsType<Systems...>>::getTickInterval() const {
		return tickInterval;
	}

	template<template<class...> class SystemsType, class... Systems>
	float32 SystemManager<SystemsType<Systems...>>::getTickAccumulation() const {
		return tickAccum;
	}
}

