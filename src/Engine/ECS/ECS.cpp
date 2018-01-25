#pragma once

// Engine
#include <Engine/ECS/ECS.hpp>
#include <Engine/Engine.hpp>

namespace Engine::ECS::detail {
	namespace ComponentData {
		decltype(nameToID) nameToID(2 * MAX_COMPONENTS);
		decltype(addComponent) addComponent;
		decltype(getComponent) getComponent;
		decltype(reclaim) reclaim;
	}

	namespace EntityData {
		decltype(componentBitsets) componentBitsets;
		decltype(alive) alive;
		decltype(reusableIDs) reusableIDs;
	}

	ComponentID getNextComponentID() {
		static ComponentID next = 0;
		return next++;
	}

	ComponentBitset& getComponentBitset(EntityID eid) {
		return detail::EntityData::componentBitsets[eid];
	}

	ComponentID getComponentID(const std::string_view name) {
		return detail::ComponentData::nameToID[name];
	}
}

namespace Engine::ECS {
	EntityID createEntity(bool forceNew) {
		auto eid = detail::EntityData::componentBitsets.size();

		if (!forceNew && !detail::EntityData::reusableIDs.empty()) {
			eid = detail::EntityData::reusableIDs.back();
			detail::EntityData::reusableIDs.pop_back();
		} else {
			detail::EntityData::componentBitsets.resize(eid + 1);
			detail::EntityData::alive.resize(eid + 1);
		}

		detail::EntityData::alive[eid] = true;

		detail::onEntityCreatedAll(eid);

		return eid;
	}

	void destroyEntity(EntityID eid) {
		detail::EntityData::reusableIDs.emplace_back(eid);
		detail::EntityData::alive[eid] = false;
		detail::EntityData::componentBitsets[eid] = 0;
		detail::onEntityDestroyedAll(eid);
	}

	bool isAlive(EntityID eid) {
		return detail::EntityData::alive[eid];
	}

	void addComponent(EntityID eid, ComponentID cid) {
		detail::ComponentData::addComponent[cid](eid, cid);
		detail::onComponentAddedAll(eid, cid);
	}


	bool hasComponent(EntityID eid, ComponentID cid) {
		return detail::getComponentBitset(eid)[cid];
	}

	bool hasComponents(EntityID eid, ComponentBitset cbits) {
		return (detail::getComponentBitset(eid) & cbits) == cbits;
	}

	void removeComponent(EntityID eid, ComponentID cid) {
		detail::getComponentBitset(eid)[cid] = false;
		detail::onComponentRemovedAll(eid, cid);
	}

	namespace {
		void topologicallySortSystems() {
			// All SystemData containers should be the same size. The use of run is arbitrary.
			const auto systemCount = detail::SystemData::run.size();

			// Sort the graph
			std::vector<int8_t> nodes(systemCount); // 0 = no mark  1 = temp mark  2 = perma mark
			std::vector<SystemID> order; // The reverse order of the systems
			order.reserve(systemCount);

			// Recursively visit all children of node using DFS
			auto visit = [&order, &nodes](SystemID node, auto& visit) {
				// Already visited
				if (nodes[node] == 2) { return; }

				// Cycle
				if (nodes[node] == 1) {
					ENGINE_ERROR("Circular system dependency involving system " << node);
				}

				// Continue visiting
				nodes[node] = 1;
				for (size_t i = 0; i < detail::SystemData::priority[node].size(); ++i) {
					if (detail::SystemData::priority[node][i]) {
						visit(i, visit);
					}
				}
				nodes[node] = 2;
				order.emplace_back(node);
			};

			// Visit all unvisited nodes
			for (size_t i = 0; i < systemCount; ++i) {
				if (nodes[i] == 0) {
					visit(i, visit);
				}
			}

			// Sort the containers
			auto reorder = [&order](auto& container) {
				std::remove_reference_t<decltype(container)> sorted;
				sorted.reserve(container.size());
				for (auto it = order.rbegin(); it != order.rend(); ++it) {
					sorted.emplace_back(container[*it]);
				}

				container = std::move(sorted);
			};

			reorder(detail::SystemData::onEntityCreated);
			reorder(detail::SystemData::onComponentAdded);
			reorder(detail::SystemData::onComponentRemoved);
			reorder(detail::SystemData::onEntityDestroyed);
			reorder(detail::SystemData::run);
		}
	}

	void init() {
		// Topological sort (DFS)
		topologicallySortSystems();

		// Shrink the containers since they should be at there final size.
		detail::SystemData::onEntityCreated.shrink_to_fit();
		detail::SystemData::onComponentAdded.shrink_to_fit();
		detail::SystemData::onComponentRemoved.shrink_to_fit();
		detail::SystemData::onEntityDestroyed.shrink_to_fit();
		detail::SystemData::run.shrink_to_fit();
	}

	void run(float dt) {
		detail::runAll(dt);
	}

	void reclaim() {
		using detail::EntityData::componentBitsets;
		using detail::EntityData::alive;
		using detail::EntityData::reusableIDs;

		reusableIDs.shrink_to_fit();

		// Attempt to reclaim from each component
		for (ComponentID cid = 0; cid < MAX_COMPONENTS; ++cid) {
			const auto func = detail::ComponentData::reclaim[cid];
			if (func == nullptr) { break; }
			func();
		}

		// Trim any dead entities
		auto pos = std::find(alive.crbegin(), alive.crend(), static_cast<uint8_t>(true));
		if (pos == alive.crend()) { return; }
		auto dist = std::distance(alive.crbegin(), pos);

		alive.erase(alive.cend() - dist, alive.cend());
		componentBitsets.erase(componentBitsets.cend() - dist, componentBitsets.cend());
		alive.shrink_to_fit();
		componentBitsets.shrink_to_fit();
	}
}
