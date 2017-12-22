#pragma once

// Engine
#include <Engine/ECS/ECS.hpp>

namespace Engine::ECS::detail {
	namespace ComponentData {
		decltype(nameToID) nameToID(2 * MAX_COMPONENTS);
		decltype(addComponent) addComponent;
		decltype(getComponent) getComponent;
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
			if (detail::EntityData::componentBitsets.size() <= eid) {
				detail::EntityData::componentBitsets.resize(eid + 1);
			}

			if (detail::EntityData::alive.size() <= eid) {
				detail::EntityData::alive.resize(eid + 1);
			}
		}

		detail::EntityData::componentBitsets[eid] = 0;
		detail::EntityData::alive[eid] = true;

		detail::onEntityCreatedAll(eid);

		return eid;
	}

	void destroyEntity(EntityID eid) {
		detail::EntityData::reusableIDs.emplace_back(eid);
		detail::EntityData::alive[eid] = false;
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
}
