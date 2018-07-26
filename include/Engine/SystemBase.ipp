// Engine
#include <Engine/SystemBase.hpp>


namespace Engine {
	template<class SystemsSet, class ComponentsSet>
	SystemBase<ECS::World<SystemsSet, ComponentsSet>>::SystemBase(World& world) : world{world} {
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onEntityAdded(ECS::Entity ent) {
	}

	template<class SystemsSet, class ComponentsSet>
	void SystemBase<ECS::World<SystemsSet, ComponentsSet>>::onEntityRemoved(ECS::Entity ent) {
	}
}
