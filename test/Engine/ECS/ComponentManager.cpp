// Engine
#include <Engine/ECS/ComponentManager.hpp>

// GoogleTest
#include<gtest/gtest.h>

namespace {
	class A {};
	class B {};
	class C {};
	class D {};
	class E {};

	class ComponentManagerTest : public testing::Test {
		public:
			ComponentManagerTest() {
				cm.registerComponent<A>("A");
				cm.registerComponent<B>("B");
				cm.registerComponent<C>("C");
				cm.registerComponent<D>("D");
				cm.registerComponent<E>("E");
			}

			Engine::ECS::ComponentManager cm;
	};
}

TEST_F(ComponentManagerTest, ComponentID) {
	Engine::ECS::ComponentManager cm2;

	cm2.registerComponent<A>("A");
	cm2.registerComponent<B>("B");
	cm2.registerComponent<C>("C");
	cm2.registerComponent<D>("D");
	cm2.registerComponent<E>("E");

	// Components registed in the same order should have the same id
	ASSERT_EQ(cm.getComponentID<A>(), cm2.getComponentID<A>());
	ASSERT_EQ(cm.getComponentID<B>(), cm2.getComponentID<B>());
	ASSERT_EQ(cm.getComponentID<C>(), cm2.getComponentID<C>());
	ASSERT_EQ(cm.getComponentID<D>(), cm2.getComponentID<D>());
	ASSERT_EQ(cm.getComponentID<E>(), cm2.getComponentID<E>());

	// Components id should be assigned to the values 0-(n-1)
	ASSERT_EQ(cm.getComponentID<A>(), 0);
	ASSERT_EQ(cm.getComponentID<B>(), 1);
	ASSERT_EQ(cm.getComponentID<C>(), 2);
	ASSERT_EQ(cm.getComponentID<D>(), 3);
	ASSERT_EQ(cm.getComponentID<E>(), 4);

	// Components names should get the correct ids
	ASSERT_EQ(cm.getComponentID("A"), 0);
	ASSERT_EQ(cm.getComponentID("B"), 1);
	ASSERT_EQ(cm.getComponentID("C"), 2);
	ASSERT_EQ(cm.getComponentID("D"), 3);
	ASSERT_EQ(cm.getComponentID("E"), 4);
}
