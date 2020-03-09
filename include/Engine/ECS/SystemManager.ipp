#pragma once

// Engine
#include <Engine/Engine.hpp>

// Meta
#include <Meta/IndexOf.hpp>


namespace Engine::ECS {
	template<template<class...> class SystemsType, class... Systems>
	template<class World>
	SystemManager<SystemsType<Systems...>>::SystemManager(World& world)
		// TODO: Constructor arguments?
		: systems{new Systems(world) ...} // TODO: Make systems stored as part of the SystemManager object (tuple for ex)
		, systemOrder{getSystemID<Systems>() ...} {

		// Update priorities
		for (SystemID sid = 0; sid < count; ++sid) {
			const auto& system = *systems[sid];
			priority[sid] |= system.priorityBefore;
		
			for (size_t i = 0; i < system.priorityAfter.size(); ++i) {
				if (system.priorityAfter[i]) {
					priority[i][sid] = true;
				}
			}
		}
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System>
	constexpr static SystemID SystemManager<SystemsType<Systems...>>::getSystemID() noexcept {
		return Meta::IndexOf<System, Systems...>::value;
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System>
	System& SystemManager<SystemsType<Systems...>>::getSystem() {
		return *static_cast<System*>(systems[getSystemID<System>()]);
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
		for (const auto& system : systems) {
			delete system;
		}
	}

	template<template<class...> class SystemsType, class... Systems>
	void SystemManager<SystemsType<Systems...>>::run(float dt) {
		for (size_t i = 0; i < count; ++i) {
			systems[systemOrder[i]]->run(dt);
		}
	}
}

