#pragma once

/**
 * @file
 * Various functions for working with color.
 * 
 * @see glm/gtc/color_space
 * @see glm/gtx/color_space
 * @see glm/gtx/color_encoding
 * @see glm/gtx/color_space_YCoCg
 */

// TODO: ideally these could be constexpr
namespace Engine::Math {
	// TODO (C++23): These functions should be constexpr-able with the now constexpr math functions.
	//               See: https://github.com/microsoft/STL/issues/2530
	//               See: https://github.com/microsoft/STL/issues/3789
	/**
	 * Converts color representation from hue, saturation, lightness to red, green, blue
	 * @param hsl Hue, saturation, and lightness in the ranges [0, 360], [0, 1], and [0, 1]
	 * @return The rgb representation in the range [0, 1]
	 */
	inline glm::vec3 cvtHSLtoRGB(const glm::vec3 hsl) {
		// TODO: Would probably be better to have all H, S, L in [0, 1] instead of H in [0, 360].
		const auto c = (1 - std::abs(2 * hsl.z - 1)) * hsl.y;
		const auto h = hsl.x * (1.0f / 60.0f);
		const auto x = c * (1 - std::abs(std::fmodf(h, 2) - 1));
		const auto m = hsl.z - c * 0.5f;

		if (h < 1) { return {c+m, x+m, 0+m}; }
		if (h < 2) { return {x+m, c+m, 0+m}; }
		if (h < 3) { return {0+m, c+m, x+m}; }
		if (h < 4) { return {0+m, x+m, c+m}; }
		if (h < 5) { return {x+m, 0+m, c+m}; }

		ENGINE_DEBUG_ASSERT(h < 6, "Invalid HSL color.");
		return {c+m, 0+m, x+m};
	}

	/** @see cvtHSLtoRGB */
	ENGINE_INLINE inline glm::vec4 cvtHSLtoRGB(const glm::vec4 hsl) {
		return {cvtHSLtoRGB({hsl.x, hsl.y, hsl.z}), hsl.w};
	}

	/**
	 * Convert from linear color to sRGB color using the 2.2 gamma approximation.
	 * For more exact conversion use glm::gt*::color_space functions
	 */
	ENGINE_INLINE inline glm::vec3 cvtApproxLinearToRGB(const glm::vec3 lin) {
		constexpr decltype(lin) exp = {1/2.2, 1/2.2, 1/2.2};
		return glm::pow(lin, exp);
	}

	/** @see cvtApproxLinearToRGB */
	ENGINE_INLINE inline glm::vec4 cvtApproxLinearToRGB(const glm::vec4 lin) {
		return {cvtApproxLinearToRGB({lin.x, lin.y, lin.z}), lin.w};
	}

	/** @see cvtApproxLinearToRGB */
	ENGINE_INLINE inline glm::vec3 cvtApproxRGBToLinear(const glm::vec3 rgb) {
		constexpr decltype(rgb) exp = {2.2, 2.2, 2.2};
		return glm::pow(rgb, exp);
	}
	
	/** @see cvtApproxLinearToRGB */
	ENGINE_INLINE inline glm::vec4 cvtApproxRGBToLinear(const glm::vec4 rgb) {
		return {cvtApproxRGBToLinear({rgb.x, rgb.y, rgb.z}), rgb.w};
	}

	/**
	 * Convert from float [0, 1] RGB to byte [0, 255] RGB representation.
	 */
	ENGINE_INLINE inline glm::u8vec3 cvtFloatRGBToByteRGB(const glm::vec3 rgb) {
		// Discussion around formulas, spacing, and error accumulation:
		// https://stackoverflow.com/questions/1914115/converting-color-value-from-float-0-1-to-byte-0-255
		return rgb * 255.0f;
	}

	/**
	 * Gets the next "random" hue such that all generated hues are equidistributed.
	 * @return The next hue in the range [0, 360].
	 * 
	 * @see https://en.wikipedia.org/wiki/Equidistributed_sequence
	 * @see https://en.wikipedia.org/wiki/Low-discrepancy_sequence#Additive_recurrence
	 */
	ENGINE_INLINE inline float32 nextRandomHue(const float32 prevHue) {
		return std::fmodf(prevHue + InvPhi<decltype(prevHue)> * 360, 360);
	}
}
