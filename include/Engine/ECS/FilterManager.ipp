#pragma once


namespace Engine::ECS {
	template<class World>
	FilterManager<World>::FilterManager() {
	}

	template<class World>
	auto FilterManager<World>::getFilterFor(const World& world, const ComponentBitset& components) -> Filter& {
		auto& filter = filters[components];

		if (!filter) {
			filter = std::make_unique<World::Filter>(world.self());
			filter->componentsBits = components;

			// Populate new filter
			const auto& entities = world.getEntities();

			for (decltype(Entity::id) eid = 0; eid < entities.size(); ++eid) {
				if (entities[eid] == 0) { continue; }

				Entity ent = Entity{eid, entities[eid]};
				filter->add(ent, world.getComponentsBitset(ent));
			}

			// Update filtersByComponentId
			for (ComponentId cid = 0; cid < components.size(); ++cid) {
				if (components.test(cid)) {
					if (filtersByComponentId.size() <= cid) {
						filtersByComponentId.resize(cid + 1);
					}

					filtersByComponentId[cid].push_back(filter.get());
				}
			}
		}

		return *filter;
	}

	
	template<class World>
	void FilterManager<World>::onComponentAdded(Entity ent, ComponentId cid, const ComponentBitset& cbits) {
		if (cid >= filtersByComponentId.size()) {
			return;
		}

		for (auto& filter : filtersByComponentId[cid]) {
			filter->add(ent, cbits);
		}
	}
	
	template<class World>
	void FilterManager<World>::onComponentRemoved(Entity ent, ComponentId cid) {
		if (cid >= filtersByComponentId.size()) { return; }

		for (auto& filter : filtersByComponentId[cid]) {
			filter->remove(ent);
		}
	}
	
	template<class World>
	void FilterManager<World>::onEntityDestroyed(Entity ent, const ComponentBitset& cbits) {
		for (auto& pair : filters) {
			auto& filterBits = pair.first;

			if ((cbits & filterBits) == filterBits) {
				pair.second->remove(ent);
			}
		}
	}
}
