#pragma once

namespace Engine::ECS::detail {
	template<class Component>
	ComponentID getComponentID() {
		const static ComponentID id = getNextComponentID();
		return id;
	}

	template<class Component>
	ComponentContainer<Component>& getComponentContainer() {
		static ComponentContainer<Component> container;
		return container;
	}

	template<class Component>
	void addComponentToEntity(EntityID eid, ComponentID cid) {
		// TODO: Currently these containers never shrink when an component is removed
		auto& container = getComponentContainer<Component>();

		if (container.size() <= eid) {
			container.resize(eid + 1);
		}

		container[eid] = Component{};
		getComponentBitset(eid)[cid] = true;
	}

	template<class Component>
	void* getComponentForEntity(EntityID eid) {
		auto& container = getComponentContainer<Component>();
		return &container[eid];
	}

	template<class Component>
	void reclaim() {
		auto& container = getComponentContainer<Component>();

		const auto cid = getComponentID<Component>();
		EntityID end = container.size();

		for (; end != 0; --end) {
			if (hasComponent(end - 1, cid)) {
				++end;
				break;
			}
		}

		container.erase(container.cbegin() + end, container.end());
		container.shrink_to_fit();
	}

	template<class Component>
	int registerComponent(const std::string_view name) {
		const auto id = getComponentID<Component>();

		if (id > MAX_COMPONENTS) {
			throw std::exception{"[Engine][ECS] Maximum number of components exceed. Increase Engine::ECS::MAX_COMPONENTS."};
		}

		detail::ComponentData::nameToID[name] = id;
		detail::ComponentData::addComponent[id] = addComponentToEntity<Component>;
		detail::ComponentData::getComponent[id] = getComponentForEntity<Component>;
		detail::ComponentData::reclaim[id] = reclaim<Component>;

		return 0;
	}
}

namespace Engine::ECS {
	template<class Component>
	void addComponent(EntityID eid) {
		addComponent(eid, detail::getComponentID<Component>());
	}

	template<class Component>
	bool hasComponent(EntityID eid) {
		return hasComponent(eid, detail::getComponentID<Component>());
	}

	template<class Component>
	void removeComponent(EntityID eid) {
		removeComponent(eid, detail::getComponentID<Component>());
	}

	template<class Component>
	Component& getComponent(EntityID eid) {
		const auto cid = detail::getComponentID<Component>();
		return *static_cast<Component*>(detail::ComponentData::getComponent[cid](eid));
	}

	template<class Component1, class Component2, class... Components>
	ComponentBitset getBitsetForComponent() {
		return getBitsetForComponent<Component1>() |= getBitsetForComponent<Component2, Components...>();
	}

	template<class Component>
	ComponentBitset getBitsetForComponent() {
		ComponentBitset value;
		value[detail::getComponentID<Component>()] = true;
		return value;
	}
}