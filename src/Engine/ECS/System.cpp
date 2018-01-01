#include <Engine/ECS/System.hpp>

namespace Engine::ECS::detail {
	namespace SystemData {
		decltype(onEntityCreated) onEntityCreated;
		decltype(onComponentAdded) onComponentAdded;
		decltype(onComponentRemoved) onComponentRemoved;
		decltype(onEntityDestroyed) onEntityDestroyed;
		decltype(run) run;
		decltype(priority) priority;
	}

	PriorityPair::PriorityPair(SystemBitset priorityBefore, SystemBitset priorityAfter)
		: priorityBefore{priorityBefore}
		, priorityAfter{priorityAfter} {
	}

	SystemID getNextSystemID() {
		static SystemID next = 0;
		return next++;
	}

	void onEntityCreatedAll(EntityID eid) {
		for (const auto func : SystemData::onEntityCreated) {
			func(eid);
		}
	}

	void onComponentAddedAll(EntityID eid, ComponentID cid) {
		for (const auto func : SystemData::onComponentAdded) {
			func(eid, cid);
		}
	}

	void onComponentRemovedAll(EntityID eid, ComponentID cid) {
		for (const auto func : SystemData::onComponentRemoved) {
			func(eid, cid);
		}
	}

	void onEntityDestroyedAll(EntityID eid) {
		for (const auto func : SystemData::onEntityDestroyed) {
			func(eid);
		}
	}

	void runAll(float dt) {
		for (const auto func : SystemData::run) {
			func(dt);
		}
	}
}