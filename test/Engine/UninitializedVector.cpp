// GoogleTest
#include <gtest/gtest.h>

// Engine
#include <Engine/UninitializedVector.hpp>


namespace {
	// The test fixture for the tests
	template<class T>
	class Engine_UninitializedVector : public ::testing::Test {
	};

	// The types to run the tests on
	using TestTypes = ::testing::Types<
		std::int8_t,
		std::int16_t,
		std::int32_t,
		std::int64_t,
		float,
		double,
		long double
	>;

	// Associate the types with the fixture
	TYPED_TEST_CASE(Engine_UninitializedVector, TestTypes);

	TYPED_TEST(Engine_UninitializedVector, Constructor) {
		Engine::UninitializedVector<TypeParam> vec;
	}

	TYPED_TEST(Engine_UninitializedVector, size_Empty) {
		Engine::UninitializedVector<TypeParam> vec;
		ASSERT_EQ(vec.size(), 0);
	}
}
