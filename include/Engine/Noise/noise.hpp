#pragma once


namespace Engine::Noise {
	/**
	 * Applies and linear congruential generator (LCG) to @p seed to generate a pseudorandom number.
	 * @tparam T The numeric type to operate on.
	 * @param seed The seed number to use for the LCG.
	 * @return A pseudorandom number.
	 */
	constexpr uint64 lcg(uint64 seed) noexcept {
		// TODO: Are these good LCG constants?
		return seed * 6364136223846793005l + 1442695040888963407l;
	}

	// TODO: move to Engine::Math, maybe combine namespaces?
	// TODO: Check if this is actually faster
	/**
	 * Takes the floor of @p x and returns it as @p Int.
	 * @tparam Int The integral type to return.
	 * @tparam Float The type of @p x.
	 * @param x The number to take the floor of.
	 * @return The floor of x as type @p Int.
	 */
	template<class Int, class Float>
	constexpr Int floorTo(Float x) noexcept {
		Int xi = static_cast<Int>(x);
		return x < xi ? xi - 1 : xi;
	}

	template<class Int, class Float>
	constexpr Int ceilTo(Float x) noexcept {
		Int xi = static_cast<Int>(x);
		return x > xi ? xi + 1 : xi;
	}

	// TODO: Write tests to verify no collision. For the time being manually checked [0, 255] in spreadsheet.
	/**
	 * Based on xorshift. Constants have been manually adjusted to values that visually seem to
	 * give good results. No idea if these are mathematically sound.
	 */
	[[nodiscard]] ENGINE_INLINE constexpr uint8 xorperm8(uint8 x) noexcept {
		x ^= x << 3;
		x ^= x >> 1;
		x ^= x << 5;
		return x;
	}
	
	// TODO: Write tests to verify no collision. For the time being manually [0, 1023] checked in spreadsheet.
	/** @copydoc xorperm8 */
	[[nodiscard]] ENGINE_INLINE constexpr uint16 xorperm16(uint16 x) noexcept {
		x ^= x << 3;
		x ^= x >> 5;
		x ^= x << 11;
		return x;
	}
}
