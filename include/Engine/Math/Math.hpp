#pragma once

// STD
#include <concepts>


namespace Engine::Math {
	template<class T, std::floating_point F>
	ENGINE_INLINE constexpr T lerp(T a, T b, F t) {
		return t * a + (F{1} - t) * b;
	}
}
