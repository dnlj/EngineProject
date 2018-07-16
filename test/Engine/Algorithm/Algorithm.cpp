// STD
#include <array>

// Google Test
#include<gtest/gtest.h>

// Engine
#include <Engine/Algorithm/Algorithm.hpp>

namespace {
	TEST(Engine_Algorithm, sort_OddSize) {
		constexpr std::array<int, 9> a = {9, 8, 7, 6, 5, 4, 3, 2, 1};
		constexpr auto b = Engine::Algorithm::sort(a);

		int i = 0;
		for (auto v : b) {
			ASSERT_EQ(++i, v);
		}
	}

	TEST(Engine_Algorithm, sort_EvenSize) {
		constexpr std::array<int, 10> a = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
		constexpr auto b = Engine::Algorithm::sort(a);

		int i = 0;
		for (auto v : b) {
			ASSERT_EQ(++i, v);
		}
	}

	TEST(Engine_Algorithm, sort_Mixed) {
		constexpr std::array<int, 10> a = {3, 9, 2, 6, 8, 4, 7, 10, 5, 1};
		constexpr auto b = Engine::Algorithm::sort(a);

		int i = 0;
		for (auto v : b) {
			ASSERT_EQ(++i, v);
		}
	}

	TEST(Engine_Algorithm, sort_ZeroSize) {
		constexpr std::array<int, 0> a = {};
		constexpr auto b = Engine::Algorithm::sort(a);

		int i = 0;
		for (auto v : b) {
			ASSERT_EQ(++i, v);
		}
	}
}
