// Engine
#include <Engine/ECS/EntityManager.hpp>

// GoogleTest
#include<gtest/gtest.h>

namespace {
	using EM = Engine::ECS::EntityManager;

	TEST(Engine_ECS_EntityManager, createEntity) {
		EM em;

		// Entities should be created sequentially
		for (int i = 0; i < 100; ++i) {
			ASSERT_EQ(i, em.createEntity());
		}
	}

	TEST(Engine_ECS_EntityManager, destroyEntity) {
		EM em;

		// Create 100 entities
		for (int i = 0; i < 100; ++i) {
			em.createEntity();
		}

		// Delete 100 entities
		for (int i = 0; i < 100; ++i) {
			em.destroyEntity(i);
		}

		// Recreate 100 entities (ids are popped in reverse order in this case)
		for (int i = 99; i >= 0; --i) {
			ASSERT_EQ(i, em.createEntity());
		}

		// Create 100 new entities
		for (int i = 100; i < 200; ++i) {
			ASSERT_EQ(i, em.createEntity());
		}
	}

	TEST(Engine_ECS_EntityManager, createEntity_forceNew) {
		EM em;

		// Create 100 entities
		for (int i = 0; i < 100; ++i) {
			em.createEntity();
		}

		// Delete 100 entities
		for (int i = 0; i < 100; ++i) {
			em.destroyEntity(i);
		}

		// Create 100 new entities
		for (int i = 100; i < 200; ++i) {
			ASSERT_EQ(i, em.createEntity(true));
		}
	}

	TEST(Engine_ECS_EntityManager, isAlive) {
		EM em;

		// Create 100 entities
		for (int i = 0; i < 100; ++i) {
			em.createEntity();
			ASSERT_TRUE(em.isAlive(i));
		}

		// Delete 100 entities
		for (int i = 0; i < 100; ++i) {
			em.destroyEntity(i);
			ASSERT_FALSE(em.isAlive(i));
		}

		// Recreate 100 entities (ids are popped in reverse order in this case)
		for (int i = 99; i >= 0; --i) {
			ASSERT_EQ(i, em.createEntity());
			ASSERT_TRUE(em.isAlive(i));
		}

		// Create 100 more entities
		for (int i = 100; i < 200; ++i) {
			ASSERT_EQ(i, em.createEntity());
			ASSERT_TRUE(em.isAlive(i));
		}
	}
}
