#pragma once

namespace Engine::ECS::detail {
	template<class System>
	System& getSystem() {
		static System sys;
		return sys;
	};

	template<class System>
	void onEntityCreated(EntityID eid) {
		getSystem<System>().onEntityCreated(Entity{eid});
	}

	template<class System>
	void onComponentAdded(EntityID eid, ComponentID cid) {
		getSystem<System>().onComponentAdded(Entity{eid}, cid);
	}

	template<class System>
	void onComponentRemoved(EntityID eid, ComponentID cid) {
		getSystem<System>().onComponentRemoved(Entity{eid}, cid);
	}

	template<class System>
	void onEntityDestroyed(EntityID eid) {
		getSystem<System>().onEntityDestroyed(Entity{eid});
	}

	template<class System>
	void run(float dt) {
		getSystem<System>().run(dt);
	}

	template<class System, class>
	int registerSystem() {
		const auto system = getSystem<System>();

		systemData.emplace_back(
			onEntityCreated<System>,
			onComponentAdded<System>,
			onComponentRemoved<System>,
			onEntityDestroyed<System>,
			run<System>
		);

		return 0;
	}
}