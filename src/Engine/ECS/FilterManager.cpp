// Engine
#include <Engine/ECS/FilterManager.hpp>


namespace Engine::ECS {
	FilterManager::FilterManager() {
		filters.max_load_factor(0.5f);
	}

	void FilterManager::onComponentAdded(Entity ent, ComponentID cid, const ComponentBitset& cbits) {
		if (cid >= filtersByComponentID.size()) { return; }

		for (auto& filter : filtersByComponentID[cid]) {
			puts("add 1");
			filter->add(ent, cbits);
		}
	}
}
