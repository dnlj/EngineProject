// Engine
#include <Engine/SystemBase.hpp>
#include <Engine/ECS/SystemManager.hpp>

// GoogleTest
#include <gtest/gtest.h>


namespace {
	constexpr size_t EXTRA_SYSTEM_COUNT = 2;

	template<int I>
	class System : public Engine::SystemBase {
		public:
			int value = 0;
			void run(float) {};
	};

	class Foo : public Engine::SystemBase {
		public:
			Foo() = delete;
			void run(float) {};

			Foo(int value1, int value2, int value3) : value1{value1}, value2{value2}, value3{value3} {};

			int value1 = 0;
			int value2 = 0;
			int value3 = 0;
	};

	class Bar : public Engine::SystemBase {
		public:
		Bar() = delete;
		void run(float) {};

		Bar(float value1, float value2) : value1{value1}, value2{value2} {};

		float value1 = 0.0f;
		float value2 = 0.0f;
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

TEST_F(SystemManagerTest, RegisterSystemArgs) {
	sm.registerSystem<Foo>(64, 32, 16);

	ASSERT_EQ(sm.getSystem<Foo>().value1, 64);
	ASSERT_EQ(sm.getSystem<Foo>().value2, 32);
	ASSERT_EQ(sm.getSystem<Foo>().value3, 16);

	sm.registerSystem<Bar>(-42.0f, -1000.0f);
	ASSERT_EQ(sm.getSystem<Bar>().value1, -42.0f);
	ASSERT_EQ(sm.getSystem<Bar>().value2, -1000.0f);
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
		constexpr auto num = Engine::ECS::MAX_SYSTEMS;

		// Generating to many systems
		recursiveRegisterWith<num>(sm2);

		ASSERT_THROW(sm2.registerSystem<System<num>>(), Engine::FatalException);
	#endif
}

TEST_F(SystemManagerTest, Sort) {
	Engine::ECS::SystemManager sm;

	int last = -1;

	class TestSystem0 : public Engine::SystemBase {
		private:
			int& last;

		public:
			TestSystem0(int& last) : last{last} {
			}

			void run(float dt) {
				ASSERT_EQ(last, 2);
				last = 0;
			};
	};

	class TestSystem1 : public Engine::SystemBase {
		private:
			int& last;

		public:
			TestSystem1(int& last) : last{last} {
				priorityBefore[2] = true;
			}

			void run(float dt) {
				ASSERT_EQ(last, -1);
				last = 1;
			};
	};

	class TestSystem2 : public Engine::SystemBase {
		private:
			int& last;

		public:
			TestSystem2(int& last) : last{last} {
				priorityBefore[0] = true;
			}

			void run(float dt) {
				ASSERT_EQ(last, 1);
				last = 2;
			};
	};

	sm.registerSystem<TestSystem0>(last);
	sm.registerSystem<TestSystem1>(last);
	sm.registerSystem<TestSystem2>(last);

	sm.sort();

	sm.run(1.0f / 60.0f);
}

TEST_F(SystemManagerTest, SortInvalid) {
	Engine::ECS::SystemManager sm;

	class TestSystem0 : public Engine::SystemBase {
		public:
			TestSystem0() {
				priorityBefore[1] = true;
			}

			void run(float dt) {};
	};

	class TestSystem1 : public Engine::SystemBase {
		public:
			TestSystem1() {
				priorityBefore[2] = true;
			}

			void run(float dt) {};
	};

	class TestSystem2 : public Engine::SystemBase {
		public:
			TestSystem2() {
				priorityBefore[0] = true;
			}

			void run(float dt) {};
	};

	sm.registerSystem<TestSystem0>();
	sm.registerSystem<TestSystem1>();
	sm.registerSystem<TestSystem2>();

	ASSERT_THROW(sm.sort(), Engine::FatalException);
}

