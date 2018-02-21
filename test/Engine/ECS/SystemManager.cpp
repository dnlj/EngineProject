// Engine
#include <Engine/SystemBase.hpp>
#include <Engine/ECS/SystemManager.hpp>

// GoogleTest
#include <gtest/gtest.h>


namespace {
	class A : public Engine::SystemBase { public: void run(float){}; };
	class B : public Engine::SystemBase { public: void run(float){}; };
	class C : public Engine::SystemBase { public: void run(float){}; };
	class D : public Engine::SystemBase { public: void run(float){}; };
	class E : public Engine::SystemBase { public: void run(float){}; };

	class SystemManagerTest : public testing::Test {
		public:
			SystemManagerTest() {
				sm.registerSystem<A>();
				sm.registerSystem<B>();
				sm.registerSystem<C>();
				sm.registerSystem<D>();
				sm.registerSystem<E>();
			}

			Engine::ECS::SystemManager sm;
	};
}

TEST_F(SystemManagerTest, SystemID) {
	Engine::ECS::SystemManager sm2;
	sm2.registerSystem<A>();
	sm2.registerSystem<B>();
	sm2.registerSystem<C>();
	sm2.registerSystem<D>();
	sm2.registerSystem<E>();

	// System ids should be assigned in sequential order (before sorting)
	ASSERT_EQ(sm.getSystemID<A>(), 0);
	ASSERT_EQ(sm.getSystemID<B>(), 1);
	ASSERT_EQ(sm.getSystemID<C>(), 2);
	ASSERT_EQ(sm.getSystemID<D>(), 3);
	ASSERT_EQ(sm.getSystemID<E>(), 4);

	// When using multiple systems
	ASSERT_EQ(sm2.getSystemID<A>(), 0);
	ASSERT_EQ(sm2.getSystemID<B>(), 1);
	ASSERT_EQ(sm2.getSystemID<C>(), 2);
	ASSERT_EQ(sm2.getSystemID<D>(), 3);
	ASSERT_EQ(sm2.getSystemID<E>(), 4);
}
