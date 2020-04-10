#pragma once


namespace Engine::ECS {
	template<class World>
	EntityFilter& FilterManager::getFilterFor(const World& world, const ComponentBitset& components) {
		auto& filter = filters[components];

		if (!filter) {
			filter = std::make_unique<EntityFilter>(entityManager);
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
				if (components[cid]) {
					if (filtersByComponentId.size() <= cid) {
						filtersByComponentId.resize(cid + 1);
					}

					filtersByComponentId[cid].push_back(filter.get());
				}
			}
		}

		return *filter;
	}
}
