#include <Engine/ECS/System.hpp>

namespace Engine::ECS::detail {
	SystemData::SystemData(
			EntityModifyFunction onEntityCreated,
			EntityModifyFunction onComponentAdded,
			EntityModifyFunction onComponentRemoved,
			EntityModifyFunction onEntityDestroyed,
			RunFunction run
		)
		: onEntityCreated{onEntityCreated}
		, onComponentAdded{onComponentAdded}
		, onComponentRemoved{onComponentRemoved}
		, onEntityDestroyed{onEntityDestroyed}
		, run{run} {
	}

	decltype(systemData) systemData;

	void onEntityCreatedAll(EntityID eid) {
		for (auto& data : systemData) {
			data.onEntityCreated(eid);
		}
	}

	void onComponentAddedAll(EntityID eid) {
		for (auto& data : systemData) {
			data.onComponentAdded(eid);
		}
	}

	void onComponentRemovedAll(EntityID eid) {
		for (auto& data : systemData) {
			data.onComponentRemoved(eid);
		}
	}

	void onEntityDestroyedAll(EntityID eid) {
		for (auto& data : systemData) {
			data.onEntityDestroyed(eid);
		}
	}

	void runAll(float dt) {
		for (auto& data : systemData) {
			data.run(dt);
		}
	}
}