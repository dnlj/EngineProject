#pragma once


namespace Game::Terrain {
	using StageId = uint8;
	using UInt = uint32;
	using Int = int32;
	using Float = float32;

	ENGINE_INLINE consteval Float operator""_f(const long double v) noexcept { return static_cast<Float>(v); }
	ENGINE_INLINE consteval Float operator""_f(const uint64 v) noexcept { return static_cast<Float>(v); }
}
