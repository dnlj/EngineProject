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
	int registerComponent(const std::string_view name) {
		const auto id = getComponentID<Component>();

		if (id > MAX_COMPONENTS) {
			throw std::exception{"[Engine][ECS] Maximum number of components exceed. Increase Engine::ECS::MAX_COMPONENTS."};
		}

		detail::componentIDMap[name] = id;
		detail::addComponentFuncitons[id] = addComponentToEntity<Component>;
		detail::getComponentFuncitons[id] = getComponentForEntity<Component>;

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
		return *static_cast<Component*>(detail::getComponentFuncitons[cid](eid));
	}
}