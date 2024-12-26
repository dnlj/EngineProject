#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Math/math.hpp>

namespace Game {
	using namespace Engine::Types;
}

namespace Game::inline Units {
	/**
	 * A position in world units (Box2D meters). These will always be relative to some origin
	 * or offset for physics and float precision reasons.
	 * Physics operates on world units.
	 */
	using WorldUnit = float32;
	using WorldVec = glm::vec<2, WorldUnit>;

	/**
	 * An approximate absolute world position in meters.
	 * This is approximate because it cannot represent fractional positions.
	 */
	using WorldAbsUnit = int64;
	using WorldAbsVec = glm::vec<2, WorldAbsUnit>;

	/**
	 * A position in blocks. Generally will be absolute positions from true (0,0),
	 * but may be an offset. These are usually combined with world units
	 * to represent the true position of an object.
	 */
	using BlockUnit = int64;
	using BlockVec = glm::vec<2, BlockUnit>;

	using ChunkUnit = BlockUnit;
	using ChunkVec = glm::vec<2, ChunkUnit>;
	using ChunkIdx = BlockVec;

	using RegionUnit = BlockUnit;
	using RegionVec = glm::vec<2, RegionUnit>;
	using RegionIdx = ChunkVec;

	using RegionBiomeUnit = ChunkUnit;
	using RegionBiomeIdx = RegionIdx;

	using ZoneId = uint32;
	constexpr inline auto zoneInvalidId = std::numeric_limits<ZoneId>::max();

	using RealmId = uint32;
}

namespace Game::inline Constants {
	constexpr inline int32 tickrate = 64;

	// Allow up to tickrate messages on the client side so that we always have
	// the most up to date inputs. Note that this is also limited by the servers
	// recvRate so it may be getting clamped anyways.
	constexpr inline int32 netrate = ENGINE_SERVER ? 21 : tickrate;

	constexpr inline int32 pixelsPerBlock = 8;
	constexpr inline int32 blocksPerMeter = 4;
	constexpr inline int32 pixelsPerMeter = pixelsPerBlock * blocksPerMeter;
	constexpr inline int32 pixelScale = 2;
	constexpr inline float32 pixelRescaleFactor = 1.0f / (pixelsPerMeter * pixelScale);
	constexpr inline float32 blockSize = 1.0f / blocksPerMeter;

	constexpr inline BlockUnit blocksPerChunk = 64;
	constexpr inline BlockVec chunkSize = {blocksPerChunk, blocksPerChunk};

	// TODO: Regions should probably be much larger, currently the active area
	//       around each player is 11x11 which is almost an entire region.
	constexpr inline RegionUnit chunksPerRegion = 16;
	constexpr inline RegionVec regionSize = {chunksPerRegion, chunksPerRegion};

	constexpr inline RegionUnit biomesPerChunk = 4;
	constexpr inline RegionVec biomesPerRegion = regionSize * biomesPerChunk;
	static_assert(blocksPerChunk % biomesPerChunk == 0, "The number of biomes per chunk should evenly divide the number of blocks per chunk.");

	// TODO: should these neighbor and zone ranges be cvars?

	/** The range at which new neighbors are added. */
	constexpr inline WorldUnit neighborRangeAdd = 5;

	/** The range at which existing neighbors will persist before being removed as neighbors. */
	constexpr inline WorldUnit neighborRangePersist = 20;

	static_assert(neighborRangePersist - neighborRangeAdd > 10, "Insufficient difference between neighbor add and persist ranges. This will cause excessive updates/work/networking.");

	/**
	 * How far away a player can be before they must be moved into a new zone.
	 * The main consideration here is the precision of calculations when using
	 * local world coordinates (WorldUnit). Box2D recommends ~2km as a maximum
	 * size before shifting (B2 docs > Overview > Units).
	 */
	constexpr WorldAbsUnit zoneMustSplitDist = 1000;

	/**
	 * How close two players can be before their zones must be merged.
	 * The main considerations here are probably the players screen size +
	 * zoom/scale and network latency to avoid entities popping in and out.
	 */
	constexpr WorldAbsUnit zoneMustJoinDist = 300;

	/**
	 * How close can two zones be to be considered the same. Used to select if
	 * an existing zone can be used for a particular group.
	 */
	constexpr WorldAbsUnit zoneSameDist = 500;

	static_assert(zoneMustSplitDist - zoneMustJoinDist > 10, "The zone split distance must be significantly larger than the join distance.");

	// If this wasn't true then you could theoretically run into instances where neighbor
	// entities are moved to both zones.
	static_assert(zoneMustJoinDist > 2*neighborRangePersist, "The zone join distance should be at minimum twice as large as the neighbor persist distance");
}

namespace Game::inline Units {
	/**
	 * Convert from a world position and an absolute offset to an approximate absolute world position.
	 */
	ENGINE_INLINE inline WorldAbsVec worldToAbsolute(const WorldVec world, const WorldAbsVec offset) noexcept {
		return offset + WorldAbsVec{glm::floor(world)};
	}

	/**
	 * Convert from an absolute world position to relative world position.
	 * @param world The absolute world position.
	 * @param offset The absolute position offset that @p world will be relative to.
	 */
	ENGINE_INLINE inline WorldAbsVec absolueToRelative(const WorldAbsVec world, const WorldAbsVec offset) noexcept {
		return world - offset;
	}

	/**
	 * Convert from a world position to a block position.
	 * @param world The local world position relative to the absolute offset @p offset.
	 * @param offset The absolute position offset that @p world uses as its origin.
	 */
	ENGINE_INLINE inline BlockVec worldToBlock(const WorldVec world, const WorldAbsVec offset) noexcept {
		// Calculate separate and then combine to correctly handle the fractional part.
		const auto o = offset * static_cast<WorldAbsUnit>(blocksPerMeter);
		const auto w = glm::floor(world * static_cast<WorldUnit>(blocksPerMeter));
		return BlockVec{o} + BlockVec{w};
	}

	/**
	 * Convert from a block position to a world position.
	 * @param block The absolute block position to convert.
	 * @param offset The absolute block position that the resulting local vector will be relative to.
	 */
	ENGINE_INLINE constexpr inline WorldVec blockToWorld(const BlockVec block, const WorldAbsVec offset) noexcept {
		const auto meters = Engine::Math::divFloor(block, static_cast<BlockUnit>(blocksPerMeter));
		const auto rem = WorldVec{meters.r} / static_cast<WorldUnit>(blocksPerMeter);
		return WorldVec{meters.q - offset} + rem;
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

	// TODO: Remove this overload when no longer used. New code should prefer the other overload.
	/**
	 * Converts from chunk coordinates to an index wrapped at increments of regionSize.
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

	/**
	 * @copybrief chunkToRegionIndex
	 * Cheaper overload for when you already know what region the chunk is in.
	 */
	ENGINE_INLINE constexpr inline RegionVec chunkToRegionIndex(const ChunkVec chunk, const RegionVec region) noexcept {
		return chunk - regionToChunk(region);
	}

	/**
	 * Converts from block coordinates to an index wrapped at increments of chunkSize.
	 */
	ENGINE_INLINE constexpr inline BlockVec blockToChunkIndex(const BlockVec block, const ChunkVec chunk) noexcept {
		return block - chunkToBlock(chunk);
	}

	/**
	 * Converts from a region index to a index into the region biomes space.
	 */
	ENGINE_INLINE constexpr inline RegionBiomeIdx regionIdxToRegionBiomeIdx(RegionIdx regionIdx) noexcept {
		return regionIdx * biomesPerChunk;
	}
}

namespace Game {
	// TODO (CMcGAzqH): this should really be a property of GetComponent<T> instead of a per-component basis.
	class MoveOnly {
		public:
			MoveOnly() = default;
			MoveOnly(MoveOnly&&) = default;
			MoveOnly& operator=(MoveOnly&&) = default;

			MoveOnly(const MoveOnly&) = delete;
			MoveOnly& operator=(const MoveOnly&) = delete;
	};
}
