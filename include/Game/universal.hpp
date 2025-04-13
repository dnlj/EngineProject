#pragma once

namespace Game {
	class UniversalChunkCoord;
	class UniversalBlockCoord;

	class UniversalRegionCoord {
		public:
			RealmId realmId;
			RegionVec pos;
			constexpr bool operator==(const UniversalRegionCoord&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoord toChunk() const noexcept;
	};

	class UniversalChunkCoord {
		public:
			RealmId realmId;
			ChunkVec pos;
			bool operator==(const UniversalChunkCoord&) const noexcept = default;
			constexpr UniversalChunkCoord operator+(const ChunkVec vec) const noexcept { return {realmId, pos + vec}; }
			ENGINE_INLINE constexpr UniversalRegionCoord toRegion() const noexcept { return { realmId, chunkToRegion(pos) }; }

			// TODO: this is questionable since we have two overloads of chunkToRegionIndex.
			ENGINE_INLINE constexpr ChunkVec toRegionIndex() const noexcept { return chunkToRegionIndex(pos); }
			ENGINE_INLINE constexpr inline UniversalBlockCoord toBlock() const noexcept;
	};

	class UniversalBlockCoord {
		public:
			RealmId realmId;
			BlockVec pos;

			constexpr UniversalBlockCoord operator+(const BlockVec vec) const noexcept { return {realmId, pos + vec}; }
			constexpr bool operator==(const UniversalBlockCoord&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoord toChunk() const noexcept { return { realmId, blockToChunk(pos) }; }
	};

	ENGINE_INLINE constexpr UniversalBlockCoord UniversalChunkCoord::toBlock() const noexcept { return { realmId, chunkToBlock(pos) }; }
	ENGINE_INLINE constexpr UniversalChunkCoord UniversalRegionCoord::toChunk() const noexcept { return { realmId, regionToChunk(pos) }; }
}

template<>
struct Engine::Hash<Game::UniversalRegionCoord> {
	[[nodiscard]]
	size_t operator()(const Game::UniversalRegionCoord& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

template<>
struct Engine::Hash<Game::UniversalChunkCoord> {
	[[nodiscard]]
	size_t operator()(const Game::UniversalChunkCoord& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

template<>
struct Engine::Hash<Game::UniversalBlockCoord> {
	[[nodiscard]]
	size_t operator()(const Game::UniversalBlockCoord& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};
