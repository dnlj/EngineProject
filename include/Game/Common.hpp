#pragma once

// Engine
#include <Engine/Engine.hpp>

namespace Game {
	using namespace Engine::Types;
	inline constexpr int32 tickrate = 64;

	// Lowering this to much will affect remote entity interpolation at high pings
	inline constexpr int32 snapshots = tickrate;
}
