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
	/**
	 * Converts color representation from hue, saturation, lightness to red, green, blue
	 * @param hsl Hue, saturation, and lightness in the ranges [0, 360], [0, 1], and [0, 1]
	 * @return The rgb representation in the range [0, 1]
	 */
	inline glm::vec3 cvtHSLtoRGB(const glm::vec3 hsl) {
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
}
