// STD
#include <iostream>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Entity.hpp>

// TODO: Add a tag system that doesnt require storage allocation (it would have to use the same component id things just not craete the arrays)
// TODO: Since we known the number of components at compile time we should be able to pre alloccate all the detail::* containers
// TODO: We never downsize any of the entity containers (entityLife, getComponentContainer)

namespace {
	class ComponentA {
		public:
			int a = -1;
			int b = -2;
	};

	class ComponentB {
		public:
			float c = -3.0f;
			float d = -4.0f;
	};

	class ComponentC {
		public:
			double e = -5.0;
			double f = -6.0;
	};
}

ENGINE_REGISTER_COMPONENT(ComponentA);
ENGINE_REGISTER_COMPONENT(ComponentB);
ENGINE_REGISTER_COMPONENT(ComponentC);



int main(int argc, char* argv[]) {
	auto ent = Engine::createEntity();

	ent.addComponent<ComponentA>();
	ent.getComponent<ComponentA>().a = 32;
	std::cout << ent.getComponent<ComponentA>().a++ << std::endl;
	std::cout << ent.getComponent<ComponentA>().a++ << std::endl;
	std::cout << ent.getComponent<ComponentA>().a++ << std::endl;
	std::cout << ent.getComponent<ComponentA>().a++ << std::endl;

	std::cout << "Done." << std::endl;
	getchar();
	return 0;
}