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

TEST_F(SystemManagerTest, BitsetForSystemsSingle) {
	{
		using Type = A;
		const auto sbits = sm.getBitsetForSystems<Type>();
		Engine::ECS::ComponentBitset bits;
		bits[sm.getSystemID<Type>()] = true;
		ASSERT_EQ(sbits, bits);
	}

	{
		using Type = B;
		const auto sbits = sm.getBitsetForSystems<Type>();
		Engine::ECS::ComponentBitset bits;
		bits[sm.getSystemID<Type>()] = true;
		ASSERT_EQ(sbits, bits);
	}

	{
		using Type = C;
		const auto sbits = sm.getBitsetForSystems<Type>();
		Engine::ECS::ComponentBitset bits;
		bits[sm.getSystemID<Type>()] = true;
		ASSERT_EQ(sbits, bits);
	}

	{
		using Type = D;
		const auto sbits = sm.getBitsetForSystems<Type>();
		Engine::ECS::ComponentBitset bits;
		bits[sm.getSystemID<Type>()] = true;
		ASSERT_EQ(sbits, bits);
	}

	{
		using Type = E;
		const auto sbits = sm.getBitsetForSystems<Type>();
		Engine::ECS::ComponentBitset bits;
		bits[sm.getSystemID<Type>()] = true;
		ASSERT_EQ(sbits, bits);
	}
}

TEST_F(SystemManagerTest, BitsetForSystemsMultiple) {
	{
		const auto sbits = sm.getBitsetForSystems<A>();
		Engine::ECS::SystemBitset bits;
		bits[sm.getSystemID<A>()] = true;
		ASSERT_EQ(sbits, bits);
	}

	{
		const auto sbits = sm.getBitsetForSystems<A, B>();
		Engine::ECS::SystemBitset bits;
		bits[sm.getSystemID<A>()] = true;
		bits[sm.getSystemID<B>()] = true;
		ASSERT_EQ(sbits, bits);
	}

	{
		const auto sbits = sm.getBitsetForSystems<A, B, C>();
		Engine::ECS::SystemBitset bits;
		bits[sm.getSystemID<A>()] = true;
		bits[sm.getSystemID<B>()] = true;
		bits[sm.getSystemID<C>()] = true;
		ASSERT_EQ(sbits, bits);
	}

	{
		const auto sbits = sm.getBitsetForSystems<A, B, C, D>();
		Engine::ECS::SystemBitset bits;
		bits[sm.getSystemID<A>()] = true;
		bits[sm.getSystemID<B>()] = true;
		bits[sm.getSystemID<C>()] = true;
		bits[sm.getSystemID<D>()] = true;
		ASSERT_EQ(sbits, bits);
	}

	{
		const auto sbits = sm.getBitsetForSystems<A, B, C, D, E>();
		Engine::ECS::SystemBitset bits;
		bits[sm.getSystemID<A>()] = true;
		bits[sm.getSystemID<B>()] = true;
		bits[sm.getSystemID<C>()] = true;
		bits[sm.getSystemID<D>()] = true;
		bits[sm.getSystemID<E>()] = true;
		ASSERT_EQ(sbits, bits);
	}
}

