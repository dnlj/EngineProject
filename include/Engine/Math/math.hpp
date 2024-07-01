#pragma once

// Windows
#if ENGINE_OS_WINDOWS
#include <intrin.h>
#endif

// STD
#include <concepts>


namespace Engine::Math {
	template<class T>
	class DivResult {
		public:
			T q{}; // Quotient
			T r{}; // Remainder
	};

	template<class T>
	ENGINE_INLINE constexpr T pow2(T num) noexcept {
		return num * num;
	}

	template<class T, std::floating_point F>
	ENGINE_INLINE constexpr T lerp(T a, T b, F t) noexcept {
		return t * a + (F{1} - t) * b;
	}

	// TODO: constexpr-ify, can switch depending on if constexpr or not
	/**
	 * Rough estimate of the inverse square root (reciprocal).
	 * @see rsqrt
	 */
	ENGINE_INLINE inline float32 rsqrt0(const float32 x) noexcept {
		return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
	}

	/**
	 * Estimate of the inverse square root (reciprocal).
	 * Uses one step of Newton's method to give a more refined result than rsqrt0.
	 * This is 2.5x as fast as using std::sqrt and very close in accuracy.
	 * See benchmark "rsqrt" for details.
	 * @see rsqrt0
	 */
	ENGINE_INLINE inline float32 rsqrt(const float32 x) noexcept {
		const auto est = rsqrt0(x);
		return est * (1.5f - x * 0.5f * est * est);
	}

	/**
	 * Rounds a integer up to the next of a given multiple.
	 */
	ENGINE_INLINE constexpr inline auto roundUpToNearest(std::integral auto const x, std::integral auto const mult) noexcept {
		return ((x + (x > 0 ? mult - 1 : 0)) / mult) * mult;
	}

	/**
	 * Calculates a "nice" number nearby of a similar scale to the given number.
	 * Nice numbers are power of ten multiples of 1, 2, and 5.
	 * Useful when displaying data such as graph labels: `spacing = niceNumber((max-min)/labelCount);`
	 * 
	 * @see "Nice Numbers for Graph Labels" by Paul S. Heckbert in "Graphics Gems"
	 */
	template<class X>
	auto niceNumber(const X x) {
		const auto u = std::pow(X(10), std::floor(std::log10(x))); // The scale of x
		const auto v = x / u; // How much of the scale we use
		if (v < decltype(u){1.5}) { return u; }
		if (v < 3) { return 2 * u; }
		if (v < 6) { return 5 * u; }
		return 10 * u;
	}

	template<std::integral T, glm::qualifier Q>
	ENGINE_INLINE constexpr auto length2(const glm::vec<2, T, Q> vec) noexcept {
		return vec.x * vec.x + vec.y * vec.y;
	}
	
	template<std::integral T, glm::qualifier Q>
	ENGINE_INLINE constexpr auto distance2(const glm::vec<2, T, Q> vecA, const glm::vec<2, T, Q> vecB) noexcept {
		return length2(vecB - vecA);
	}

	/**
	 * Integer division + floor
	 * @param num The numerator to use.
	 * @param den The positive denominator to use.
	 */
	template<std::integral T>
	[[nodiscard]] ENGINE_INLINE constexpr auto divFloor(T num, T den) noexcept {
		// There are a number of ways to implement this. This is the fastest
		// (see Bench, divfloor) and IMO the most readable. Good assembly
		// between all MSVC/GCC/Clang as a bonus compared to other implementations.
		const auto q = num / den;
		const auto r = num % den;

		// If the result is negative and there is a remainder, subtract one (floor).
		// If it is positive the truncation is already the same as floor.
		const auto q0 = q - (r ? num < 0 : 0);
		const auto r0 = num - den * q0; // TODO: is there a way to get this based on the original r? I don't think so because we also do the floor after.

		return DivResult<T>{ .q = q0, .r = r0 };
	}

	template<std::integral T, glm::qualifier Q>
	[[nodiscard]] ENGINE_INLINE constexpr auto divFloor(glm::vec<2, T, Q> num, T den) noexcept {
		const auto x = divFloor(num.x, den);
		const auto y = divFloor(num.y, den);
		return DivResult<decltype(num)>{
			.q = {x.q, y.q},
			.r = {x.r, y.r},
		};
	}

	/**
	 * Integer division + ceil
	 * @param num The numerator to use.
	 * @param den The positive denominator to use.
	 *
	 * @see divFloor
	 */
	template<std::integral T>
	[[nodiscard]] ENGINE_INLINE constexpr auto divCeil(T num, T den) noexcept {
		const auto q = num / den;
		const auto r = num % den;
		const auto q0 = q + (r ? num > 0 : 0);
		const auto r0 = num - den * q0;
		return DivResult<T>{ .q = q0, .r = r0 };
	}
	
	template<std::integral T, glm::qualifier Q>
	[[nodiscard]] ENGINE_INLINE constexpr auto divCeil(glm::vec<2, T, Q> num, T den) noexcept {
		const auto x = divCeil(num.x, den);
		const auto y = divCeil(num.y, den);
		return DivResult<decltype(num)>{
			.q = {x.q, y.q},
			.r = {x.r, y.r},
		};
	}
}
