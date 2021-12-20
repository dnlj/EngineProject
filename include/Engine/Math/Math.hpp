#pragma once
// Windows
#include <intrin.h>

// STD
#include <concepts>


namespace Engine::Math {
	template<class T, std::floating_point F>
	ENGINE_INLINE constexpr T lerp(T a, T b, F t) {
		return t * a + (F{1} - t) * b;
	}

	/**
	 * Rough estimate of the inverse square root.
	 * @see rsqrt
	 */
	ENGINE_INLINE float rsqrt0(const float x) {
		return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
	}

	/**
	 * Estimate of the inverse square root.
	 * Uses one step of Newton's method to give a more refined result than rsqrt0.
	 * This is 2.5x as fast as using std::sqrt and very close in accuracy.
	 * See benchmark "rsqrt" for details.
	 * @see rsqrt0
	 */
	ENGINE_INLINE float rsqrt(const float x) {
		const auto est = rsqrt0(x);
		return est * (1.5f - x * 0.5f * est * est);
	}
}
