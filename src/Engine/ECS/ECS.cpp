#pragma once

// Engine
#include <Engine/ECS/ECS.hpp>

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
			if (detail::EntityData::componentBitsets.size() <= eid) {
				detail::EntityData::componentBitsets.resize(eid + 1);
			}

			if (detail::EntityData::alive.size() <= eid) {
				detail::EntityData::alive.resize(eid + 1);
			}
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

	void init() {
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
