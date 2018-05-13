#pragma once

namespace Engine::ECS {
	template<class SystemsSet, class ComponentsSet>
	EntityID World<SystemsSet, ComponentsSet>::createEntity(bool forceNew) {
		const auto eid = EntityManager::createEntity(forceNew);

		if (eid > componentBitsets.size()) {
			componentBitsets.resize(eid + 1);
		}
	}

	template<class SystemsSet, class ComponentsSet>
	ComponentBitset& World<SystemsSet, ComponentsSet>::getComponentBitset(EntityID eid) {
		return componentBitsets[eid];
	}

	template<class SystemsSet, class ComponentsSet>
	const ComponentBitset& World<SystemsSet, ComponentsSet>::getComponentBitset(EntityID eid) const {
		return getComponentBitset(eid);
	}
}
