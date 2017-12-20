#pragma once

namespace Engine {
	template<class Component>
	void Entity::addComponent() {
		ECS::addComponent<Component>(eid);
	};

	template<class Component>
	bool Entity::hasComponent() const {
		return ECS::hasComponent<Component>(eid);
	}

	template<class Component>
	void Entity::removeComponent() {
		ECS::removeComponent<Component>(eid);
	}

	template<class Component>
	Component& Entity::getComponent() {
		return ECS::getComponent<Component>(eid);
	}
}