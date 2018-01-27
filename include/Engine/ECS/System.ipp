#pragma once

namespace Engine::ECS {
	template<class System1, class System2, class... Systems>
	SystemBitset getBitsetForSystems() {
		return getBitsetForSystems<System1>() |= getBitsetForSystems<System2, Systems...>();
	}

	template<class System>
	SystemBitset getBitsetForSystems() {
		SystemBitset value;
		value[detail::getSystemID<System>()] = true;
		return value;
	}
}

namespace Engine::ECS::detail {
	template<class System>
	System& getSystem() {
		static System sys;
		return sys;
	};

	template<class System>
	SystemID getSystemID() {
		const static SystemID id = getNextSystemID();
		return id;
	}

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
		const auto& system = getSystem<System>();
		const auto sid = getSystemID<System>();

		// All containers should be of the same size
		if (sid >= SystemData::run.size()) {
			SystemData::onEntityCreated.resize(sid + 1);
			SystemData::onComponentAdded.resize(sid + 1);
			SystemData::onComponentRemoved.resize(sid + 1);
			SystemData::onEntityDestroyed.resize(sid + 1);
			SystemData::run.resize(sid + 1);
		}

		SystemData::onEntityCreated[sid] = onEntityCreated<System>;
		SystemData::onComponentAdded[sid] = onComponentAdded<System>;
		SystemData::onComponentRemoved[sid] = onComponentRemoved<System>;
		SystemData::onEntityDestroyed[sid] = onEntityDestroyed<System>;
		SystemData::run[sid] = run<System>;


		// Update priorities
		SystemData::priority[sid] |= system.priorityBefore;
		
		for (size_t i = 0; i < system.priorityAfter.size(); ++i) {
			if (system.priorityAfter[i]) {
				SystemData::priority[i][sid] = true;
			}
		}

		return 0;
	}
}
