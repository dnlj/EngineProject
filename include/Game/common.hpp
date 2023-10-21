#pragma once

// Engine
#include <Engine/Engine.hpp>

namespace Game {
	using namespace Engine::Types;
	constexpr inline int32 tickrate = 64;

	constexpr inline int32 pixelsPerBlock = 8;
	constexpr inline int32 blocksPerMeter = 4;
	constexpr inline int32 pixelsPerMeter = pixelsPerBlock * blocksPerMeter;
	constexpr inline int32 pixelScale = 2;
	constexpr inline float32 pixelRescaleFactor = 1.0f / (pixelsPerMeter * pixelScale);
	constexpr inline float32 blockSize = 1.0f / blocksPerMeter;

	using BlockUnit = int64;
	using BlockVec = glm::vec<2, BlockUnit>;
	using WorldUnit = float32;
	using WorldVec = glm::vec<2, WorldUnit>;
	//static_assert(std::same_as<WorldVec, glm::vec2>);
	//static_assert(std::same_as<WorldUnit, decltype(b2Vec2::x)>);

	ENGINE_INLINE inline BlockVec worldToBlock(const WorldVec world, const BlockVec offset) noexcept {
		return offset + BlockVec{glm::floor(world / blockSize)};
	}

	ENGINE_INLINE inline WorldVec blockToWorld(const BlockVec block, const BlockVec offset) noexcept {
		return WorldVec{block - offset} * blockSize;
	}
}
