#pragma once


namespace Engine::Noise {
	// TODO: name
	// TODO: split
	/**
	 * Applies and linear congruential generator (LCG) to @p seed to generate a pseudorandom number.
	 * @tparam T The numeric type to operate on.
	 * @param seed The seed number to use for the LCG.
	 * @return A pseudorandom number.
	 */
	template<class T>
	constexpr T lcg(T seed) {
		// TODO: Are these good LCG constants?
		return seed * 6364136223846793005l + 1442695040888963407l;
	}

	// TODO: split
	// TODO: Check if this is actually faster
	/**
	 * Takes the floor of @p x and returns it as @p Int.
	 * @tparam Int The integral type to return.
	 * @tparam Float The type of @p x.
	 * @param x The number to take the floor of.
	 * @return The floor of x as type @p Int.
	 */
	template<class Int, class Float>
	constexpr Int floorTo(Float x) {
		Int xi = static_cast<Int>(x);
		return x < xi ? xi - 1 : xi;
	}
}
