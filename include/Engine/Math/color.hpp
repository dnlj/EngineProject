#pragma once


namespace Engine::Math {
	/**
	 * Converts color representation from hue, saturation, lightness to red, green, blue
	 * @param hsl Hue, saturation, and lightness in the ranges [0, 360], [0, 1], and [0, 1]
	 * @return The rgb representation in the range [0, 1]
	 */
	inline glm::vec3 cvtHSLtoRGB(const glm::vec3 hsl2) {
		const auto hsl = glm::dvec3(hsl2);
		const float64 c = (1 - std::abs(2 * hsl.z - 1)) * hsl.y;
		const float64 h = hsl.x * (1.0 / 60.0);
		const float64 x = c * (1 - std::abs(std::fmod(h, 2) - 1));
		const float64 m = hsl.z - c * 0.5;

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
}
