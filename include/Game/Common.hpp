#pragma once

// Engine
#include <Engine/Engine.hpp>

namespace Game {
	using namespace Engine::Types;
	inline constexpr int32 tickrate = 64;

	inline constexpr int32 pixelsPerBlock = 8;
	inline constexpr int32 blocksPerMeter = 4;
	inline constexpr int32 pixelsPerMeter = pixelsPerBlock * blocksPerMeter;
}
