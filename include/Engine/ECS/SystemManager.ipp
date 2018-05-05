#pragma once

// Engine
#include <Engine/Engine.hpp>

// Meta
#include <Meta/IndexOf.hpp>


namespace Engine::ECS {
	template<template<class...> class SystemsType, class... Systems>
	SystemManager<SystemsType<Systems...>>::SystemManager()
		// TODO: Constructor arguments?
		: systems{new Systems() ...}
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

		// Sort systems
		sort();
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System>
	constexpr SystemID SystemManager<SystemsType<Systems...>>::getSystemID() noexcept {
		return Meta::IndexOf<System, Systems...>::value;
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System>
	System& SystemManager<SystemsType<Systems...>>::getSystem() {
		return *static_cast<System*>(systems[getSystemID<System>()]);
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System1, class System2, class... SystemN>
	SystemBitset SystemManager<SystemsType<Systems...>>::getBitsetForSystems() {
		return getBitsetForSystems<System1>() |= (getBitsetForSystems<System2>() |= ... |= getBitsetForSystems<SystemN>());
	}

	template<template<class...> class SystemsType, class... Systems>
	template<class System1>
	SystemBitset SystemManager<SystemsType<Systems...>>::getBitsetForSystems() {
		const auto sid = getSystemID<System1>();
		SystemBitset value;
		value[sid] = true;
		return value;
	}

	template<template<class...> class SystemsType, class... Systems>
	SystemManager<SystemsType<Systems...>>::~SystemManager() {
		for (const auto& system : systems) {
			if (system != nullptr) {
				delete system;
			}
		}
	}

	template<template<class...> class SystemsType, class... Systems>
	void SystemManager<SystemsType<Systems...>>::onEntityCreated(EntityID eid) {
		for (size_t i = 0; i < count; ++i) {
			systems[systemOrder[i]]->onEntityCreated(eid);
		}
	}

	template<template<class...> class SystemsType, class... Systems>
	void SystemManager<SystemsType<Systems...>>::onComponentAdded(EntityID eid, ComponentID cid) {
		for (size_t i = 0; i < count; ++i) {
			systems[systemOrder[i]]->onComponentAdded(eid, cid);
		}
	}

	template<template<class...> class SystemsType, class... Systems>
	void SystemManager<SystemsType<Systems...>>::onComponentRemoved(EntityID eid, ComponentID cid) {
		for (size_t i = 0; i < count; ++i) {
			systems[systemOrder[i]]->onComponentRemoved(eid, cid);
		}
	}

	template<template<class...> class SystemsType, class... Systems>
	void SystemManager<SystemsType<Systems...>>::onEntityDestroyed(EntityID eid) {
		for (size_t i = 0; i < count; ++i) {
			systems[systemOrder[i]]->onEntityDestroyed(eid);
		}
	}

	template<template<class...> class SystemsType, class... Systems>
	void SystemManager<SystemsType<Systems...>>::run(float dt) {
		for (size_t i = 0; i < count; ++i) {
			systems[systemOrder[i]]->run(dt);
		}
	}

	template<template<class...> class SystemsType, class... Systems>
	void SystemManager<SystemsType<Systems...>>::sort() {
		// Sort the graph
		std::vector<int8_t> nodes(count); // 0 = no mark  1 = temp mark  2 = perma mark
		std::vector<SystemID> order; // The reverse order of the systems
		order.reserve(count);

		// Recursively visit all children of node using DFS
		auto visit = [this, &order, &nodes](SystemID node, auto& visit) {
			// Already visited
			if (nodes[node] == 2) { return; }

			// Cycle
			if (nodes[node] == 1) {
				ENGINE_ERROR("Circular system dependency involving system " << node);
			}

			// Continue visiting
			nodes[node] = 1;

			for (size_t i = 0; i < priority[node].size(); ++i) {
				if (priority[node][i]) {
					visit(i, visit);
				}
			}

			nodes[node] = 2;
			order.emplace_back(node);
		};

		// Visit all unvisited nodes
		for (size_t i = 0; i < count; ++i) {
			if (nodes[i] == 0) {
				visit(i, visit);
			}
		}

		// Sort the containers
		auto reorder = [this, &order](auto& container) {
			std::remove_reference_t<decltype(container)> sorted{};

			for (size_t i = count; --i != static_cast<size_t>(-1);) {
				sorted[count - i - 1] = std::move(container[order[i]]);
			}

			container = std::move(sorted);
		};

		// Do the sorting
		reorder(systemOrder);
	}
}

