// Engine
#include <Engine/ECS/World.hpp>

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// GoogleTest
#include <gtest/gtest.h>

namespace {
	template<int I>
	class Component {
		public:
			int value = 0;
	};

	template<int>
	class System;
	
	using ComponentA = Component<0>;
	using ComponentB = Component<1>;
	using ComponentC = Component<2>;
	using ComponentD = Component<3>;
	using ComponentE = Component<4>;

	using SystemA = System<0>;
	using SystemB = System<1>;
	using SystemC = System<2>;
	using SystemD = System<3>;
	using SystemE = System<4>;

	using SystemsSet = Meta::TypeSet::TypeSet<
		SystemA,
		SystemB,
		SystemC,
		SystemD,
		SystemE
	>;

	using ComponentsSet = Meta::TypeSet::TypeSet<
		ComponentA,
		ComponentB,
		ComponentC,
		ComponentD,
		ComponentE
	>;

	using World = Engine::ECS::World<SystemsSet, ComponentsSet>;

	template<int I>
	class System : public Engine::ECS::System {
		public:
			System(World&) {};
			int value = 0;
	};
}

namespace {
	TEST(Engine_ECS_World, create_destroy_Entity) {
		World w;

		auto ent = w.createEntity();
		w.destroyEntity(ent);
	}

	TEST(Engine_ECS_World, getComponent) {
		World w;

		auto ent = w.createEntity();
		
		auto& c1 = w.addComponent<ComponentB>(ent);
		auto& c2 = w.getComponent<ComponentB>(ent);

		ASSERT_EQ(&c1, &c2);

		auto& c3 = w.getComponent<ComponentB>(ent);

		ASSERT_EQ(&c2, &c3);
	}

	TEST(Engine_ECS_World, has_add_remove_Component) {
		World w;
		auto ent = w.createEntity();

		ASSERT_FALSE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentB>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentD>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentE>(ent));

		w.addComponent<ComponentA>(ent);

		ASSERT_TRUE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentB>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentD>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentE>(ent));

		w.addComponent<ComponentC>(ent);

		ASSERT_TRUE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentB>(ent));
		ASSERT_TRUE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentD>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentE>(ent));

		w.addComponent<ComponentE>(ent);

		ASSERT_TRUE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentB>(ent));
		ASSERT_TRUE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentD>(ent));
		ASSERT_TRUE(w.hasComponent<ComponentE>(ent));

		w.removeComponent<ComponentA>(ent);

		ASSERT_FALSE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentB>(ent));
		ASSERT_TRUE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentD>(ent));
		ASSERT_TRUE(w.hasComponent<ComponentE>(ent));

		w.removeComponent<ComponentC>(ent);

		ASSERT_FALSE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentB>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentD>(ent));
		ASSERT_TRUE(w.hasComponent<ComponentE>(ent));

		w.removeComponent<ComponentE>(ent);

		ASSERT_FALSE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentB>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentD>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentE>(ent));
	}

	TEST(Engine_ECS_World, hasComponent) {
		World w;

		const auto ent = w.createEntity();

		ASSERT_FALSE(w.hasComponent(ent, w.getComponentID<ComponentA>()));
		ASSERT_FALSE(w.hasComponent(ent, w.getComponentID<ComponentC>()));
		ASSERT_FALSE(w.hasComponent(ent, w.getComponentID<ComponentE>()));

		w.addComponent<ComponentA>(ent);
		w.addComponent<ComponentC>(ent);
		w.addComponent<ComponentE>(ent);

		ASSERT_TRUE(w.hasComponent(ent, w.getComponentID<ComponentA>()));
		ASSERT_TRUE(w.hasComponent(ent, w.getComponentID<ComponentC>()));
		ASSERT_TRUE(w.hasComponent(ent, w.getComponentID<ComponentE>()));

		w.removeComponent<ComponentA>(ent);
		w.removeComponent<ComponentC>(ent);
		w.removeComponent<ComponentE>(ent);

		ASSERT_FALSE(w.hasComponent(ent, w.getComponentID<ComponentA>()));
		ASSERT_FALSE(w.hasComponent(ent, w.getComponentID<ComponentC>()));
		ASSERT_FALSE(w.hasComponent(ent, w.getComponentID<ComponentE>()));
	}

	TEST(Engine_ECS_World, hasComponent_Template) {
		World w;

		const auto ent = w.createEntity();

		ASSERT_FALSE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentE>(ent));

		w.addComponent<ComponentA>(ent);
		w.addComponent<ComponentC>(ent);
		w.addComponent<ComponentE>(ent);

		ASSERT_TRUE(w.hasComponent<ComponentA>(ent));
		ASSERT_TRUE(w.hasComponent<ComponentC>(ent));
		ASSERT_TRUE(w.hasComponent<ComponentE>(ent));

		w.removeComponent<ComponentA>(ent);
		w.removeComponent<ComponentC>(ent);
		w.removeComponent<ComponentE>(ent);

		ASSERT_FALSE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentE>(ent));
	}

	TEST(Engine_ECS_World, hasComponents) {
		World w;

		const auto ent = w.createEntity();
		w.addComponent<ComponentA>(ent);
		w.addComponent<ComponentC>(ent);
		w.addComponent<ComponentE>(ent);

		Engine::ECS::ComponentBitset cbits;
		cbits[0] = true;
		cbits[2] = true;
		cbits[4] = true;

		ASSERT_TRUE(w.hasComponents(ent, cbits));
	}

	TEST(Engine_ECS_World, hasComponents_Template) {
		World w;

		const auto ent = w.createEntity();
		w.addComponents<ComponentA, ComponentC, ComponentE>(ent);

		const auto value = w.hasComponents<ComponentA, ComponentC, ComponentE>(ent);
		ASSERT_TRUE(value);
	}

	TEST(Engine_ECS_World, addComponents) {
		World w;

		const auto ent = w.createEntity();
		auto& [a, c, e] = w.addComponents<ComponentA, ComponentC, ComponentE>(ent);
		const auto cbits = w.getBitsetForComponents<ComponentA, ComponentC, ComponentE>();

		ASSERT_TRUE(w.hasComponents(ent, cbits));
		ASSERT_TRUE(&a == &w.getComponent<ComponentA>(ent));
		ASSERT_TRUE(&c == &w.getComponent<ComponentC>(ent));
		ASSERT_TRUE(&e == &w.getComponent<ComponentE>(ent));
	}

	TEST(Engine_ECS_World, removeComponents) {
		World w;

		const auto ent = w.createEntity();
		const auto cbits = w.getBitsetForComponents<ComponentA, ComponentC, ComponentE>();

		w.addComponents<ComponentA, ComponentC, ComponentE>(ent);

		ASSERT_TRUE(w.hasComponents(ent, cbits));

		w.removeComponents<ComponentA, ComponentC, ComponentE>(ent);

		ASSERT_FALSE(w.hasComponent<ComponentA>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentC>(ent));
		ASSERT_FALSE(w.hasComponent<ComponentE>(ent));
	}

	TEST(Engine_ECS_World, getComponents) {
		World w;

		const auto ent = w.createEntity();
		auto& [a1, c1, e1] = w.addComponents<ComponentA, ComponentC, ComponentE>(ent);
		auto& [a2, c2, e2] = w.getComponents<ComponentA, ComponentC, ComponentE>(ent);


		ASSERT_TRUE(&a1 == &a2);
		ASSERT_TRUE(&c1 == &c2);
		ASSERT_TRUE(&e1 == &e2);
	}

	TEST(Engine_ECS_World, getComponentsBitset) {
		World w;

		const auto ent = w.createEntity();
		w.addComponents<ComponentA, ComponentC, ComponentE>(ent);

		auto cbits = w.getComponentsBitset(ent);

		for (std::size_t i = 0; i < cbits.size(); ++i) {
			if (i == w.getComponentID<ComponentA>()
				|| i == w.getComponentID<ComponentC>()
				|| i == w.getComponentID<ComponentE>()) {

				ASSERT_TRUE(cbits[i]);
			} else {
				ASSERT_FALSE(cbits[i]);
			}
		}
	}
}
