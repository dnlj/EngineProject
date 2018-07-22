#pragma once


namespace Engine::ECS {
	template<class World>
	EntityFilter& FilterManager::getFilterFor(const World& world, const ComponentBitset& components) {
		auto& filter = filters[components];

		if (!filter) {
			filter = std::make_unique<EntityFilter>();
			filter->componentsBits = components;

			// TODO: Populate new filter

			// Update filtersByComponentID
			for (ComponentID cid = 0; cid < components.size(); ++cid) {
				if (components[cid]) {
					if (filtersByComponentID.size() <= cid) {
						filtersByComponentID.resize(cid + 1);
					}

					filtersByComponentID[cid].push_back(filter.get());
				}
			}
		}

		return *filter;
	}
}
