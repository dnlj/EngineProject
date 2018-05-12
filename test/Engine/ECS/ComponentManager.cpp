// Engine
#include <Engine/ECS/ComponentManager.hpp>
#include <Engine/FatalException.hpp>

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// GoogleTest
#include<gtest/gtest.h>

namespace {
	class A {};
	class B {};
	class C {};
	class D {};
	class E {};
	class Nonregistered {};

	using CM = Engine::ECS::ComponentManager<
		Meta::TypeSet::TypeSet<A, B, C, D, E>
	>;

	TEST(Engine_ECS_ComponentManager, getComponentID) {
		CM cm;

		// Components id should be assigned sequentially in order
		ASSERT_EQ(cm.getComponentID<A>(), 0);
		ASSERT_EQ(cm.getComponentID<B>(), 1);
		ASSERT_EQ(cm.getComponentID<C>(), 2);
		ASSERT_EQ(cm.getComponentID<D>(), 3);
		ASSERT_EQ(cm.getComponentID<E>(), 4);
	}

	TEST(Engine_ECS_ComponentManager, getBitsetForComponents_Single) {
		CM cm;

		{
			using Type = A;
			const auto cbits = cm.getBitsetForComponents<Type>();
			Engine::ECS::ComponentBitset bits;
			bits[cm.getComponentID<Type>()] = true;
			ASSERT_EQ(cbits, bits);
		}

		{
			using Type = B;
			const auto cbits = cm.getBitsetForComponents<Type>();
			Engine::ECS::ComponentBitset bits;
			bits[cm.getComponentID<Type>()] = true;
			ASSERT_EQ(cbits, bits);
		}

		{
			using Type = C;
			const auto cbits = cm.getBitsetForComponents<Type>();
			Engine::ECS::ComponentBitset bits;
			bits[cm.getComponentID<Type>()] = true;
			ASSERT_EQ(cbits, bits);
		}

		{
			using Type = D;
			const auto cbits = cm.getBitsetForComponents<Type>();
			Engine::ECS::ComponentBitset bits;
			bits[cm.getComponentID<Type>()] = true;
			ASSERT_EQ(cbits, bits);
		}

		{
			using Type = E;
			const auto cbits = cm.getBitsetForComponents<Type>();
			Engine::ECS::ComponentBitset bits;
			bits[cm.getComponentID<Type>()] = true;
			ASSERT_EQ(cbits, bits);
		}
	}

	TEST(Engine_ECS_ComponentManager, getBitsetForComponents_Multiple) {
		CM cm;

		const auto cbits = cm.getBitsetForComponents<A, C, E>();
		Engine::ECS::ComponentBitset bits;

		bits[cm.getComponentID<A>()] = true;
		bits[cm.getComponentID<C>()] = true;
		bits[cm.getComponentID<E>()] = true;

		ASSERT_EQ(cbits, bits);
	}

	TEST(Engine_ECS_ComponentManager, getComponentContainer) {
		CM cm;

		{
			using T = A;

			ASSERT_EQ(cm.getComponentContainer<T>().size(), 0);
			
			cm.getComponentContainer<T>().push_back(T{});
			ASSERT_EQ(cm.getComponentContainer<T>().size(), 1);

			cm.getComponentContainer<T>().push_back(T{});
			ASSERT_EQ(cm.getComponentContainer<T>().size(), 2);

			cm.getComponentContainer<T>().push_back(T{});
			ASSERT_EQ(cm.getComponentContainer<T>().size(), 3);
		}

		{
			using T = C;

			ASSERT_EQ(cm.getComponentContainer<T>().size(), 0);
			
			cm.getComponentContainer<T>().push_back(T{});
			ASSERT_EQ(cm.getComponentContainer<T>().size(), 1);

			cm.getComponentContainer<T>().push_back(T{});
			ASSERT_EQ(cm.getComponentContainer<T>().size(), 2);

			cm.getComponentContainer<T>().push_back(T{});
			ASSERT_EQ(cm.getComponentContainer<T>().size(), 3);
		}

		{
			using T = E;

			ASSERT_EQ(cm.getComponentContainer<T>().size(), 0);
			
			cm.getComponentContainer<T>().push_back(T{});
			ASSERT_EQ(cm.getComponentContainer<T>().size(), 1);

			cm.getComponentContainer<T>().push_back(T{});
			ASSERT_EQ(cm.getComponentContainer<T>().size(), 2);

			cm.getComponentContainer<T>().push_back(T{});
			ASSERT_EQ(cm.getComponentContainer<T>().size(), 3);
		}
	}
}