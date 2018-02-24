// Engine
#include <Engine/ECS/SystemManager.hpp>

#if defined(DEBUG)
	#include <Engine/Engine.hpp>
#endif

// Static
namespace Engine::ECS {
	SystemID SystemManager::getNextGlobalSystemID() {
		static SystemID id = 0;

		#if defined(DEBUG)
			if (id >= MAX_SYSTEMS_GLOBAL) {
				ENGINE_ERROR("Attempting to generate invalid global system id.");
			}
		#endif

		return id++;
	}
}

namespace Engine::ECS {
	SystemManager::SystemManager() {
		std::fill(globalToLocalID.begin(), globalToLocalID.end(), static_cast<SystemID>(-1));
	}

	SystemID SystemManager::getNextSystemID() {
		#if defined(DEBUG)
			if (nextID >= MAX_SYSTEMS) {
				ENGINE_ERROR("Attempting to generate an invalid local system id.");
			}
		#endif

		return nextID++;
	}

	SystemID SystemManager::getSystemID(SystemID gsid) {
		const auto sid = globalToLocalID[gsid];

		#if defined(DEBUG)
			if (sid >= nextID) {
				ENGINE_ERROR("Attempting to get the local id of an nonregistered system.");
			}
		#endif

		return sid;
	}

	void SystemManager::onEntityCreated(EntityID eid) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.onEntityCreated[i])(eid);
		}
	}

	void SystemManager::onComponentAdded(EntityID eid, ComponentID cid) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.onComponentAdded[i])(eid, cid);
		}
	}

	void SystemManager::onComponentRemoved(EntityID eid, ComponentID cid) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.onComponentRemoved[i])(eid, cid);
		}
	}

	void SystemManager::onEntityDestroyed(EntityID eid) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.onEntityDestroyed[i])(eid);
		}
	}

	void SystemManager::run(float dt) {
		for (size_t i = 0; i < systems.count; ++i) {
			(this->*systems.run[i])(dt);
		}
	}

	void SystemManager::sort() {
		// Sort the graph
		std::vector<int8_t> nodes(systems.count); // 0 = no mark  1 = temp mark  2 = perma mark
		std::vector<SystemID> order; // The reverse order of the systems
		order.reserve(systems.count);

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

			for (size_t i = 0; i < systems.priority[node].size(); ++i) {
				if (systems.priority[node][i]) {
					visit(i, visit);
				}
			}

			nodes[node] = 2;
			order.emplace_back(node);
		};

		// Visit all unvisited nodes
		for (size_t i = 0; i < systems.count; ++i) {
			if (nodes[i] == 0) {
				visit(i, visit);
			}
		}

		// Sort the containers
		auto reorder = [this, &order](auto& container) {
			std::remove_reference_t<decltype(container)> sorted;

			for (size_t i = systems.count; --i != static_cast<size_t>(-1);) {
				sorted[systems.count - i - 1] = std::move(container[order[i]]);
			}

			container = std::move(sorted);
		};

		// Do the sorting
		reorder(systems.onEntityCreated);
		reorder(systems.onEntityDestroyed);
		reorder(systems.onComponentAdded);
		reorder(systems.onComponentRemoved);
		reorder(systems.run);
		reorder(systems.priority);

		for (size_t i = 0; i < systems.count; ++i) {
			reorder(systems.priority[i]);
		}

		// Update gsid to sid translation
		{
			decltype(globalToLocalID) localToGlobalID;

			for(size_t i = 0; i < globalToLocalID.size(); ++i) {
				if (globalToLocalID[i] != static_cast<SystemID>(-1)) {
					localToGlobalID[globalToLocalID[i]] = i;
				}
			}

			for (size_t i = systems.count; --i != static_cast<size_t>(-1);) {
				const auto idx = systems.count - i - 1;
				globalToLocalID[localToGlobalID[idx]] = order[i];
			}
		}
	}
}
