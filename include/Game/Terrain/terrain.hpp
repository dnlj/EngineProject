#pragma once

#include <glm/glm.hpp>


namespace Game::Terrain {
	using UInt = uint32;
	using Int = int32;
	using Float = float32;
	using FVec2 = glm::vec<2, Float>;

	using StageId = uint8;
	using BiomeId = uint8;

	ENGINE_INLINE consteval Float operator""_f(const long double v) noexcept { return static_cast<Float>(v); }
	ENGINE_INLINE consteval Float operator""_f(const uint64 v) noexcept { return static_cast<Float>(v); }
}
