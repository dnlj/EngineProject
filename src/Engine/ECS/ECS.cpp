#pragma once

// Engine
#include <Engine/ECS/ECS.hpp>

namespace Engine::ECS::detail {
	decltype(componentIDMap) componentIDMap(2 * MAX_COMPONENTS);
	decltype(addComponentFuncitons) addComponentFuncitons;
	decltype(getComponentFuncitons) getComponentFuncitons;
	decltype(entityComponentBitsets) entityComponentBitsets;
	decltype(entityLife) entityLife;
	decltype(reusableEntityIDs) reusableEntityIDs;

	ComponentID getNextComponentID() {
		static ComponentID next = 0;
		return next++;
	}

	ComponentBitset& getComponentBitset(EntityID eid) {
		return detail::entityComponentBitsets[eid];
	}

	ComponentID getComponentID(const std::string_view name) {
		return detail::componentIDMap[name];
	}
}

namespace Engine::ECS {
	EntityID createEntity(bool forceNew) {
		auto eid = detail::entityComponentBitsets.size();

		if (!forceNew && !detail::reusableEntityIDs.empty()) {
			eid = detail::reusableEntityIDs.front();
			detail::reusableEntityIDs.pop();
		} else {
			if (detail::entityComponentBitsets.size() <= eid) {
				detail::entityComponentBitsets.resize(eid + 1);
			}

			if (detail::entityLife.size() <= eid) {
				detail::entityLife.resize(eid + 1);
			}
		}

		detail::entityComponentBitsets[eid] = 0;
		detail::entityLife[eid] = true;
		return eid;
	}

	void destroyEntity(EntityID eid) {
		detail::reusableEntityIDs.push(eid);
		detail::entityLife[eid] = false;
	}

	bool isAlive(EntityID eid) {
		return detail::entityLife[eid];
	}

	void addComponent(EntityID eid, ComponentID cid) {
		detail::addComponentFuncitons[cid](eid, cid);
	}


	bool hasComponent(EntityID eid, ComponentID cid) {
		return detail::getComponentBitset(eid)[cid];
	}

	

	void removeComponent(EntityID eid, ComponentID cid) {
		detail::getComponentBitset(eid)[cid] = false;
	}
}
