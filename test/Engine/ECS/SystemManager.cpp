// Engine
#include <Engine/SystemBase.hpp>
#include <Engine/ECS/SystemManager.hpp>

// GoogleTest
#include <gtest/gtest.h>


namespace {
	template<int I>
	class System : public Engine::SystemBase {
		public:
			int value = 0;
			void run(float) {};
	};

	using A = System<0>;
	using B = System<1>;
	using C = System<2>;
	using D = System<3>;
	using E = System<4>;
	using Nonregistered = System<-1>;

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

TEST_F(SystemManagerTest, GetSystem) {
	// Values are correctly initialized
	ASSERT_EQ(sm.getSystem<A>().value, 0);
	ASSERT_EQ(sm.getSystem<B>().value, 0);
	ASSERT_EQ(sm.getSystem<C>().value, 0);
	ASSERT_EQ(sm.getSystem<D>().value, 0);
	ASSERT_EQ(sm.getSystem<E>().value, 0);

	// Values are changed
	sm.getSystem<A>().value = 1;
	sm.getSystem<B>().value = 2;
	sm.getSystem<C>().value = 4;

	ASSERT_EQ(sm.getSystem<A>().value, 1);
	ASSERT_EQ(sm.getSystem<B>().value, 2);
	ASSERT_EQ(sm.getSystem<C>().value, 4);

	// Values are changed again
	sm.getSystem<A>().value = 8;
	sm.getSystem<B>().value = 16;
	sm.getSystem<C>().value = 32;

	ASSERT_EQ(sm.getSystem<A>().value, 8);
	ASSERT_EQ(sm.getSystem<B>().value, 16);
	ASSERT_EQ(sm.getSystem<C>().value, 32);
}

TEST_F(SystemManagerTest, SystemIDThrows) {
	// Getting the id of a nonregistered system
	#if defined(DEBUG)
		ASSERT_THROW(sm.getSystemID<Nonregistered>(), Engine::FatalException);
	#endif
}

namespace {
	template<int I>
	void recursiveRegisterWith(Engine::ECS::SystemManager& sm) {
		sm.registerSystem<System<I - 1>>();
		recursiveRegisterWith<I - 1>(sm);
	}

	template<>
	void recursiveRegisterWith<1>(Engine::ECS::SystemManager& sm) {
		sm.registerSystem<System<0>>();
	}
}

TEST_F(SystemManagerTest, ToManySystemsThrows) {
	#if defined(DEBUG)
		Engine::ECS::SystemManager sm2;

		// Generating to many systems
		recursiveRegisterWith<Engine::ECS::MAX_SYSTEMS - 1>(sm2);

		ASSERT_THROW(sm2.registerSystem<System<Engine::ECS::MAX_SYSTEMS - 1>>(), Engine::FatalException);
	#endif
}
