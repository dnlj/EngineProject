// STD
#include <algorithm>
#include <iostream>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Entity.hpp>
#include <Engine/SystemBase.hpp>

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


namespace {
	class SystemA : public Engine::SystemBase {
		public:
			SystemA() {
				cbits[Engine::ECS::detail::getComponentID<ComponentA>()] = true;
				cbits[Engine::ECS::detail::getComponentID<ComponentB>()] = true;
			}

			void run(float dt) {
				std::cout << "A Run: " << dt << std::endl;
				
				for (auto& ent : entities) {
					std::cout << "A process: " << ent.getID() << "\n";
				}
			};
	};

	class SystemB : public Engine::SystemBase {
		public:
			void onEntityCreated(Engine::Entity ent) {
				std::cout << "B create: " << ent << std::endl;
			}

			void onComponentAdded(Engine::Entity ent, Engine::ECS::ComponentID cid) {
				std::cout << "B add: " << ent << " " << cid << std::endl;
			}

			void onComponentRemoved(Engine::Entity ent, Engine::ECS::ComponentID cid) {
				std::cout << "B remove: " << ent << " " << cid << std::endl;
			}

			void onEntityDestroyed(Engine::Entity ent) {
				std::cout << "B destroy: " << ent << std::endl;
			}

			void run(float dt) {
				std::cout << "B Run: " << dt << std::endl;
			};
	};
}

ENGINE_REGISTER_SYSTEM(SystemA);
ENGINE_REGISTER_SYSTEM(SystemB);


int main(int argc, char* argv[]) {
	auto e0 = Engine::createEntity();
	auto e1 = Engine::createEntity();
	auto e2 = Engine::createEntity();

	e0.addComponent<ComponentA>();
	Engine::ECS::detail::runAll(0.1666f);
	e0.addComponent<ComponentC>();
	Engine::ECS::detail::runAll(0.1666f);
	e0.addComponent<ComponentB>();
	Engine::ECS::detail::runAll(0.1666f);
	e0.removeComponent<ComponentB>();
	Engine::ECS::detail::runAll(0.1666f);
	e0.addComponent<ComponentB>();
	Engine::ECS::detail::runAll(0.1666f);
	Engine::destroyEntity(e0);
	Engine::ECS::detail::runAll(0.1666f);


	std::cout << "Done." << std::endl;
	getchar();
	return 0;
}