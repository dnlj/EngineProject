// Engine
#include <Engine/ECS/ECS.hpp>

// Google Test
#include <gtest/gtest.h>

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


// Since ECS is global it isnt easily split into multiple tests
TEST(EngineECSTest, ECSTest) {
	Engine::ECS::EntityID eid;

	// Create entity
	{
		eid = Engine::ECS::createEntity();

		EXPECT_EQ(Engine::ECS::isAlive(eid), true);
	}

	// Add/Remove components
	{
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentA>(eid), false);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentB>(eid), false);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentC>(eid), false);

		Engine::ECS::addComponent<ComponentB>(eid);

		EXPECT_EQ(Engine::ECS::hasComponent<ComponentA>(eid), false);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentB>(eid), true);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentC>(eid), false);

		Engine::ECS::addComponent<ComponentA>(eid);

		EXPECT_EQ(Engine::ECS::hasComponent<ComponentA>(eid), true);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentB>(eid), true);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentC>(eid), false);

		Engine::ECS::addComponent<ComponentC>(eid);

		EXPECT_EQ(Engine::ECS::hasComponent<ComponentA>(eid), true);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentB>(eid), true);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentC>(eid), true);

		Engine::ECS::removeComponent<ComponentB>(eid);

		EXPECT_EQ(Engine::ECS::hasComponent<ComponentA>(eid), true);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentB>(eid), false);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentC>(eid), true);

		Engine::ECS::removeComponent<ComponentA>(eid);

		EXPECT_EQ(Engine::ECS::hasComponent<ComponentA>(eid), false);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentB>(eid), false);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentC>(eid), true);

		Engine::ECS::removeComponent<ComponentC>(eid);

		EXPECT_EQ(Engine::ECS::hasComponent<ComponentA>(eid), false);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentB>(eid), false);
		EXPECT_EQ(Engine::ECS::hasComponent<ComponentC>(eid), false);
	}

	// Destroy entity
	{
		Engine::ECS::destroyEntity(eid);

		EXPECT_EQ(Engine::ECS::isAlive(eid), false);
	}

	// Reuse entity id
	{
		// Dont re-use
		auto e2 = Engine::ECS::createEntity(true);
		EXPECT_NE(eid, e2);

		// Do re-use
		auto e3 = Engine::ECS::createEntity();
		EXPECT_EQ(eid, e3);

		// Life
		EXPECT_EQ(Engine::ECS::isAlive(e2), true);
		EXPECT_EQ(Engine::ECS::isAlive(e3), true);

		Engine::ECS::destroyEntity(e2);

		EXPECT_EQ(Engine::ECS::isAlive(e2), false);
		EXPECT_EQ(Engine::ECS::isAlive(e3), true);

		Engine::ECS::destroyEntity(e3);

		EXPECT_EQ(Engine::ECS::isAlive(e2), false);
		EXPECT_EQ(Engine::ECS::isAlive(e3), false);
	}
}