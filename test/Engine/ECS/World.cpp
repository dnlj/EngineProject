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

		auto eid = w.createEntity();
		w.destroyEntity(eid);
	}

	TEST(Engine_ECS_World, getComponent) {
		World w;

		auto eid = w.createEntity();
		
		auto& c1 = w.addComponent<ComponentB>(eid);
		auto& c2 = w.getComponent<ComponentB>(eid);

		ASSERT_EQ(&c1, &c2);

		auto& c3 = w.getComponent<ComponentB>(eid);

		ASSERT_EQ(&c2, &c3);
	}

	TEST(Engine_ECS_World, has_add_remove_Component) {
		World w;
		auto eid = w.createEntity();

		ASSERT_FALSE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentB>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentD>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentE>(eid));

		w.addComponent<ComponentA>(eid);

		ASSERT_TRUE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentB>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentD>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentE>(eid));

		w.addComponent<ComponentC>(eid);

		ASSERT_TRUE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentB>(eid));
		ASSERT_TRUE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentD>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentE>(eid));

		w.addComponent<ComponentE>(eid);

		ASSERT_TRUE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentB>(eid));
		ASSERT_TRUE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentD>(eid));
		ASSERT_TRUE(w.hasComponent<ComponentE>(eid));

		w.removeComponent<ComponentA>(eid);

		ASSERT_FALSE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentB>(eid));
		ASSERT_TRUE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentD>(eid));
		ASSERT_TRUE(w.hasComponent<ComponentE>(eid));

		w.removeComponent<ComponentC>(eid);

		ASSERT_FALSE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentB>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentD>(eid));
		ASSERT_TRUE(w.hasComponent<ComponentE>(eid));

		w.removeComponent<ComponentE>(eid);

		ASSERT_FALSE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentB>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentD>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentE>(eid));
	}

	TEST(Engine_ECS_World, hasComponent) {
		World w;

		const auto eid = w.createEntity();

		ASSERT_FALSE(w.hasComponent(eid, w.getComponentID<ComponentA>()));
		ASSERT_FALSE(w.hasComponent(eid, w.getComponentID<ComponentC>()));
		ASSERT_FALSE(w.hasComponent(eid, w.getComponentID<ComponentE>()));

		w.addComponent<ComponentA>(eid);
		w.addComponent<ComponentC>(eid);
		w.addComponent<ComponentE>(eid);

		ASSERT_TRUE(w.hasComponent(eid, w.getComponentID<ComponentA>()));
		ASSERT_TRUE(w.hasComponent(eid, w.getComponentID<ComponentC>()));
		ASSERT_TRUE(w.hasComponent(eid, w.getComponentID<ComponentE>()));

		w.removeComponent<ComponentA>(eid);
		w.removeComponent<ComponentC>(eid);
		w.removeComponent<ComponentE>(eid);

		ASSERT_FALSE(w.hasComponent(eid, w.getComponentID<ComponentA>()));
		ASSERT_FALSE(w.hasComponent(eid, w.getComponentID<ComponentC>()));
		ASSERT_FALSE(w.hasComponent(eid, w.getComponentID<ComponentE>()));
	}

	TEST(Engine_ECS_World, hasComponent_Template) {
		World w;

		const auto eid = w.createEntity();

		ASSERT_FALSE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentE>(eid));

		w.addComponent<ComponentA>(eid);
		w.addComponent<ComponentC>(eid);
		w.addComponent<ComponentE>(eid);

		ASSERT_TRUE(w.hasComponent<ComponentA>(eid));
		ASSERT_TRUE(w.hasComponent<ComponentC>(eid));
		ASSERT_TRUE(w.hasComponent<ComponentE>(eid));

		w.removeComponent<ComponentA>(eid);
		w.removeComponent<ComponentC>(eid);
		w.removeComponent<ComponentE>(eid);

		ASSERT_FALSE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentE>(eid));
	}

	TEST(Engine_ECS_World, hasComponents) {
		World w;

		const auto eid = w.createEntity();
		w.addComponent<ComponentA>(eid);
		w.addComponent<ComponentC>(eid);
		w.addComponent<ComponentE>(eid);

		Engine::ECS::ComponentBitset cbits;
		cbits[0] = true;
		cbits[2] = true;
		cbits[4] = true;

		ASSERT_TRUE(w.hasComponents(eid, cbits));
	}

	TEST(Engine_ECS_World, hasComponents_Template) {
		World w;

		const auto eid = w.createEntity();
		w.addComponents<ComponentA, ComponentC, ComponentE>(eid);

		const auto value = w.hasComponents<ComponentA, ComponentC, ComponentE>(eid);
		ASSERT_TRUE(value);
	}

	TEST(Engine_ECS_World, addComponents) {
		World w;

		const auto eid = w.createEntity();
		auto& [a, c, e] = w.addComponents<ComponentA, ComponentC, ComponentE>(eid);
		const auto cbits = w.getBitsetForComponents<ComponentA, ComponentC, ComponentE>();

		ASSERT_TRUE(w.hasComponents(eid, cbits));
		ASSERT_TRUE(&a == &w.getComponent<ComponentA>(eid));
		ASSERT_TRUE(&c == &w.getComponent<ComponentC>(eid));
		ASSERT_TRUE(&e == &w.getComponent<ComponentE>(eid));
	}

	TEST(Engine_ECS_World, removeComponents) {
		World w;

		const auto eid = w.createEntity();
		const auto cbits = w.getBitsetForComponents<ComponentA, ComponentC, ComponentE>();

		w.addComponents<ComponentA, ComponentC, ComponentE>(eid);

		ASSERT_TRUE(w.hasComponents(eid, cbits));

		w.removeComponents<ComponentA, ComponentC, ComponentE>(eid);

		ASSERT_FALSE(w.hasComponent<ComponentA>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentC>(eid));
		ASSERT_FALSE(w.hasComponent<ComponentE>(eid));
	}

	TEST(Engine_ECS_World, getComponents) {
		World w;

		const auto eid = w.createEntity();
		auto& [a1, c1, e1] = w.addComponents<ComponentA, ComponentC, ComponentE>(eid);
		auto& [a2, c2, e2] = w.getComponents<ComponentA, ComponentC, ComponentE>(eid);


		ASSERT_TRUE(&a1 == &a2);
		ASSERT_TRUE(&c1 == &c2);
		ASSERT_TRUE(&e1 == &e2);
	}
}
