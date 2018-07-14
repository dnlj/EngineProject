// STD
#include <vector>

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
			const auto ent = em.createEntity();
			ASSERT_EQ(i, ent.id);
			ASSERT_EQ(1, ent.gen);
		}
	}

	TEST(Engine_ECS_EntityManager, destroyEntity) {
		EM em;

		std::vector<Engine::ECS::Entity> ents(100);

		// Create 100 entities
		for (int i = 0; i < 100; ++i) {
			ents[i] = em.createEntity();
		}

		// Delete 100 entities
		for (int i = 0; i < 100; ++i) {
			em.destroyEntity(ents[i]);
		}

		// Recreate 100 entities (ids are popped in reverse order in this case)
		for (int i = 99; i >= 0; --i) {
			ents[i] = em.createEntity();

			ASSERT_EQ(i, ents[i].id);
			ASSERT_EQ(2, ents[i].gen);
		}

		// Create 100 new entities
		for (int i = 100; i < 200; ++i) {
			const auto ent = em.createEntity();
			ASSERT_EQ(i, ent.id);
			ASSERT_EQ(1, ent.gen);
		}
	}

	TEST(Engine_ECS_EntityManager, createEntity_forceNew) {
		EM em;

		std::vector<Engine::ECS::Entity> ents(100);

		// Create 100 entities
		for (int i = 0; i < 100; ++i) {
			ents[i] = em.createEntity();
		}

		// Delete 100 entities
		for (int i = 0; i < 100; ++i) {
			em.destroyEntity(ents[i]);
		}

		// Create 100 new entities
		for (int i = 100; i < 200; ++i) {
			const auto ent = em.createEntity(true);
			ASSERT_EQ(i, ent.id);
			ASSERT_EQ(1, ent.gen);
		}
	}

	TEST(Engine_ECS_EntityManager, isAlive) {
		EM em;

		std::vector<Engine::ECS::Entity> ents(100);

		// Create 100 entities
		for (int i = 0; i < 100; ++i) {
			ents[i] = em.createEntity();
			ASSERT_TRUE(em.isAlive(ents[i]));
		}

		// Delete 100 entities
		for (int i = 0; i < 100; ++i) {
			em.destroyEntity(ents[i]);
			ASSERT_FALSE(em.isAlive(ents[i]));
		}

		// Recreate 100 entities (ids are popped in reverse order in this case)
		for (int i = 99; i >= 0; --i) {
			ASSERT_TRUE(em.isAlive(em.createEntity()));
		}

		// Create 100 more entities
		for (int i = 100; i < 200; ++i) {
			ASSERT_TRUE(em.isAlive(em.createEntity()));
		}
	}
}
