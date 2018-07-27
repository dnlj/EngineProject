// Engine
#include <Engine/SystemBase.hpp>


namespace Engine {
	template<class SystemsSet, class ComponentsSet>
	SystemBase<ECS::World<SystemsSet, ComponentsSet>>::SystemBase(World& world) : world{world} {
	}
}
