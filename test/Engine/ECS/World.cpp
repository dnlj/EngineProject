// Engine
#include <Engine/ECS/World.hpp>

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// GoogleTest
#include <gtest/gtest.h>

// TODO: Fix
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
}
