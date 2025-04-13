#pragma once

// STD
#include <concepts>

// Engine
#include <Engine/Engine.hpp>

// TODO: Move to Math/bit.hpp
namespace Engine::Bit {
	/**
	 * Counts the number of trailing zeros in an integer.
	 */
	template<class I>
	constexpr I ctz(I i) { // TODO: c++20 concepts std::integral. Currently breaks VS intellisense
		// TODO: Prolly better way to impl
		I c = 0;
		while (i ^ 1) { ++c; i = i >> 1; }
		return c;
	}
}
