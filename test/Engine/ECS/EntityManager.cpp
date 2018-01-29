// Engine
#include <Engine/ECS/EntityManager.hpp>

// GoogleTest
#include<gtest/gtest.h>

class EntityManagerTest : public testing::Test {
	public:
		Engine::ECS::EntityManager em;
};

TEST_F(EntityManagerTest, CreateEntity) {
	// Entities should be created sequentially
	for (int i = 0; i < 100; ++i) {
		ASSERT_EQ(i, em.createEntity());
	}
}

TEST_F(EntityManagerTest, CreateDestroyEntity) {
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

	// Create 100 more entities
	for (int i = 100; i < 200; ++i) {
		ASSERT_EQ(i, em.createEntity());
	}
}

TEST_F(EntityManagerTest, CreateNewEntity) {
	// Create 100 entities
	for (int i = 0; i < 100; ++i) {
		em.createEntity();
	}

	// Delete 100 entities
	for (int i = 0; i < 100; ++i) {
		em.destroyEntity(i);
	}

	// Create 100 entities
	for (int i = 100; i < 200; ++i) {
		ASSERT_EQ(i, em.createEntity(true));
	}
}

TEST_F(EntityManagerTest, EntityAlive) {
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

