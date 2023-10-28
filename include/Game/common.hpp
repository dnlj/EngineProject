#pragma once

// Engine
#include <Engine/Engine.hpp>

namespace Game {
	using namespace Engine::Types;

	using WorldUnit = float32;
	using WorldVec = glm::vec<2, WorldUnit>;

	using BlockUnit = int64;
	using BlockVec = glm::vec<2, BlockUnit>;

	using ChunkUnit = BlockUnit;
	using ChunkVec = glm::vec<2, ChunkUnit>;

	using RegionUnit = BlockUnit;
	using RegionVec = glm::vec<2, RegionUnit>;
	
	//static_assert(std::same_as<WorldVec, glm::vec2>);
	//static_assert(std::same_as<WorldUnit, decltype(b2Vec2::x)>);


	constexpr inline int32 tickrate = 64;

	constexpr inline int32 pixelsPerBlock = 8;
	constexpr inline int32 blocksPerMeter = 4;
	constexpr inline int32 pixelsPerMeter = pixelsPerBlock * blocksPerMeter;
	constexpr inline int32 pixelScale = 2;
	constexpr inline float32 pixelRescaleFactor = 1.0f / (pixelsPerMeter * pixelScale);
	constexpr inline float32 blockSize = 1.0f / blocksPerMeter;

	constexpr inline BlockUnit blocksPerChunk = 64;
	constexpr inline BlockVec chunkSize = {blocksPerChunk, blocksPerChunk};

	constexpr inline RegionUnit chunksPerRegion = 16;
	constexpr inline RegionVec regionSize = {chunksPerRegion, chunksPerRegion};

	/**
	 * Convert from a world position to a block position.
	 * @param world The local world position relative to the absolute offset @offset.
	 * @param offset The absolute position offset that @p world uses as its origin.
	 */
	ENGINE_INLINE constexpr inline BlockVec worldToBlock(const WorldVec world, const BlockVec offset) noexcept {
		return offset + BlockVec{glm::floor(world / blockSize)};
	}

	/**
	 * Convert from a block position to a world position.
	 * @param block The absolute block position to convert.
	 * @param offset The absolute block position that the resulting local vector will be relative to.
	 */
	ENGINE_INLINE constexpr inline WorldVec blockToWorld(const BlockVec block, const BlockVec offset) noexcept {
		return WorldVec{block - offset} * blockSize;
	}

	/**
	 * Converts from block coordinates to chunk coordinates.
	 */
	ENGINE_INLINE constexpr inline ChunkVec blockToChunk(const BlockVec block) noexcept {
		// Integer division + floor
		auto d = block / chunkSize;
		d.x = d.x * chunkSize.x == block.x ? d.x : d.x - (block.x < 0);
		d.y = d.y * chunkSize.y == block.y ? d.y : d.y - (block.y < 0);
		return d;
	}

	/**
	 * Converts from chunk coordinates to block coordinates.
	 */
	ENGINE_INLINE constexpr inline BlockVec chunkToBlock(const ChunkVec chunk) noexcept {
		return chunk * chunkSize;
	}

	/**
	 * Converts from chunk coordinates to region coordinates.
	 */
	ENGINE_INLINE constexpr inline RegionVec chunkToRegion(const ChunkVec chunk) noexcept {
		// Integer division + floor
		auto d = chunk / regionSize;
		d.x = d.x * regionSize.x == chunk.x ? d.x : d.x - (chunk.x < 0);
		d.y = d.y * regionSize.y == chunk.y ? d.y : d.y - (chunk.y < 0);
		return d;
	}

	/**
	 * Converts from chunk coordinates to an index wrapped at increments of MapRegion::size.
	 */
	ENGINE_INLINE constexpr inline RegionVec chunkToRegionIndex(const ChunkVec chunk) noexcept {
		return (regionSize + chunk % regionSize) % regionSize;
	}

	/**
	 * Converts from region coordinates to chunk coordinates.
	 */
	ENGINE_INLINE constexpr inline ChunkVec regionToChunk(const RegionVec region) noexcept {
		return region * regionSize;
	}
}
