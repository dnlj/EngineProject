// Engine
#include <Engine/ECS/SystemManager.hpp>
#include <Engine/ECS/World.hpp>

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// GoogleTest
#include <gtest/gtest.h>


namespace {
	template<int>
	class System;
	
	using A = System<0>;
	using B = System<1>;
	using C = System<2>;
	using D = System<3>;
	using E = System<4>;
	
	using SystemsSet = Meta::TypeSet::TypeSet<A, B, C, D, E>;
	using ComponentsSet = Meta::TypeSet::TypeSet<>;
	
	using SM = Engine::ECS::SystemManager<SystemsSet>;
	using World = Engine::ECS::World<SystemsSet, ComponentsSet>;
	
	template<int I>
	class System : public Engine::ECS::System {
		public:
			System(World&){};
			int value = 0;
	};
	
	//class Foo : public Engine::ECS::System {
	//	public:
	//		Foo() = delete;
	//		Foo(int value1, int value2, int value3) : value1{value1}, value2{value2}, value3{value3} {};
	//
	//		int value1 = 0;
	//		int value2 = 0;
	//		int value3 = 0;
	//};
	//
	//class Bar : public Engine::ECS::System {
	//	public:
	//		Bar() = delete;
	//		Bar(float value1, float value2) : value1{value1}, value2{value2} {};
	//
	//		float value1 = 0.0f;
	//		float value2 = 0.0f;
	//};
	
	// TODO: Reimplement using new methods if the test still makes sense
	//TEST_F(SystemManagerTest, RegisterSystemArgs) {
	//	sm.registerSystem<Foo>(64, 32, 16);
	//
	//	ASSERT_EQ(sm.getSystem<Foo>().value1, 64);
	//	ASSERT_EQ(sm.getSystem<Foo>().value2, 32);
	//	ASSERT_EQ(sm.getSystem<Foo>().value3, 16);
	//
	//	sm.registerSystem<Bar>(-42.0f, -1000.0f);
	//	ASSERT_EQ(sm.getSystem<Bar>().value1, -42.0f);
	//	ASSERT_EQ(sm.getSystem<Bar>().value2, -1000.0f);
	//}
	
	TEST(Engine_ECS_SystemManager, getSystemID) {
		World w;
		SM sm(w);
		
		// System ids should be assigned in sequential order (before sorting)
		ASSERT_EQ(sm.getSystemID<A>(), 0);
		ASSERT_EQ(sm.getSystemID<B>(), 1);
		ASSERT_EQ(sm.getSystemID<C>(), 2);
		ASSERT_EQ(sm.getSystemID<D>(), 3);
		ASSERT_EQ(sm.getSystemID<E>(), 4);
	}
	
	TEST(Engine_ECS_SystemManager, getBitsetForSystems_Single) {
		World w;
		SM sm{w};
		
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
	
	TEST(Engine_ECS_SystemManager, getBitsetForSystems_Multiple) {
		World w;
		SM sm{w};
		
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
	
	TEST(Engine_ECS_SystemManager, getSystem) {
		World w;
		SM sm{w};
		
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
	
	TEST(Engine_ECS_SystemManager, sort) {
		int last = -1;

		class TestSystem0;
		class TestSystem1;
		class TestSystem2;
		
		using SystemsSet = Meta::TypeSet::TypeSet<
			TestSystem0,
			TestSystem1,
			TestSystem2
		>;
		
		using SM = Engine::ECS::SystemManager<SystemsSet>;
		using World = Engine::ECS::World<SystemsSet, ComponentsSet>;
		
		class TestSystem0 : public Engine::ECS::System {
			public:
				int* last;
				
				TestSystem0(World&) {
				}
				
				void run(float dt) {
					ASSERT_EQ(*last, 2);
					*last = 0;
				};
		};
	
		class TestSystem1 : public Engine::ECS::System {
			public:
				int* last;
				
				TestSystem1(World&) {
					priorityBefore[2] = true;
				}
				
				void run(float dt) {
					ASSERT_EQ(*last, -1);
					*last = 1;
				};
		};
	
		class TestSystem2 : public Engine::ECS::System {
			public:
				int* last;
				
				TestSystem2(World&) {
					priorityBefore[0] = true;
				}
				
				void run(float dt) {
					ASSERT_EQ(*last, 1);
					*last = 2;
				};
		};
		
		World w;
		SM sm{w};
		
		sm.getSystem<TestSystem0>().last = &last;
		sm.getSystem<TestSystem1>().last = &last;
		sm.getSystem<TestSystem2>().last = &last;
		
		sm.run(1.0f / 60.0f);
	}
	
	TEST(Engine_ECS_SystemManager, sort_Invalid) {
		class TestSystem0;
		class TestSystem1;
		class TestSystem2;
		
		using SystemsSet = Meta::TypeSet::TypeSet<
			TestSystem0,
			TestSystem1,
			TestSystem2
		>;
		
		using SM = Engine::ECS::SystemManager<SystemsSet>;
		using World = Engine::ECS::World<SystemsSet, ComponentsSet>;

		class TestSystem0 : public Engine::ECS::System {
			public:
				TestSystem0(World&) {
					priorityBefore[1] = true;
				}
				
				void run(float dt) {};
		};
		
		class TestSystem1 : public Engine::ECS::System {
			public:
				TestSystem1(World&) {
					priorityBefore[2] = true;
				}
				
				void run(float dt) {};
		};
		
		class TestSystem2 : public Engine::ECS::System {
			public:
				TestSystem2(World&) {
					priorityBefore[0] = true;
				}
				
				void run(float dt) {};
		};
		
		ASSERT_THROW(SM{World{}}, Engine::FatalException);
	}
}
