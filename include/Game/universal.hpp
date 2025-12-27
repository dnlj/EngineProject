#pragma once

namespace Game {
	class UniversalChunkCoord;
	class UniversalChunkCoordX;
	class UniversalBlockCoord;
	class UniversalBlockCoordX;

	class UniversalRegionCoordX {
		public:
			RealmId realmId;
			RegionUnit pos;
			ENGINE_INLINE constexpr UniversalChunkCoordX toChunk() const noexcept;
			constexpr bool operator==(const UniversalRegionCoordX&) const noexcept = default;
	};

	class UniversalRegionCoord {
		public:
			RealmId realmId;
			RegionVec pos;
			constexpr bool operator==(const UniversalRegionCoord&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoord toChunk() const noexcept;
			ENGINE_INLINE constexpr UniversalRegionCoordX toX() const noexcept { return { realmId, pos.x }; }
	};

	class UniversalChunkCoordX {
		public:
			RealmId realmId;
			ChunkUnit pos;

			ENGINE_INLINE constexpr UniversalRegionCoordX toRegion() const noexcept { return { realmId, chunkToRegion({pos, 0}).x }; }
			ENGINE_INLINE constexpr UniversalBlockCoordX toBlock() const noexcept;
			constexpr UniversalChunkCoordX operator+(const UniversalChunkCoordX other) const noexcept { return {realmId, pos + other.pos}; }
			constexpr UniversalChunkCoordX operator-(const UniversalChunkCoordX other) const noexcept { return {realmId, pos - other.pos}; }
	};

	class UniversalChunkCoord {
		public:
			RealmId realmId;
			ChunkVec pos;

			ENGINE_INLINE constexpr UniversalChunkCoordX toX() const noexcept { return { realmId, pos.x }; }

			bool operator==(const UniversalChunkCoord&) const noexcept = default;
			constexpr UniversalChunkCoord operator+(const ChunkVec vec) const noexcept { return {realmId, pos + vec}; }

			ENGINE_INLINE constexpr UniversalRegionCoord toRegion() const noexcept { return { realmId, chunkToRegion(pos) }; }

			// TODO: this is questionable since we have two overloads of chunkToRegionIndex.
			ENGINE_INLINE constexpr ChunkVec toRegionIndex() const noexcept { return chunkToRegionIndex(pos); }
			ENGINE_INLINE constexpr ChunkVec toRegionIndex(UniversalRegionCoord regionCoord) const noexcept { return chunkToRegionIndex(pos, regionCoord.pos); }
			ENGINE_INLINE constexpr inline UniversalBlockCoord toBlock() const noexcept;
	};

	class UniversalBlockCoordX {
		public:
			RealmId realmId;
			BlockUnit pos;

			ENGINE_INLINE constexpr UniversalBlockCoordX operator+(const BlockUnit off) const noexcept { return {realmId, pos + off}; }
			ENGINE_INLINE constexpr UniversalBlockCoordX operator-(const BlockUnit off) const noexcept { return {realmId, pos - off}; }
	};

	class UniversalBlockCoord {
		public:
			RealmId realmId;
			BlockVec pos;

			ENGINE_INLINE constexpr UniversalBlockCoord operator+(const BlockVec vec) const noexcept { return {realmId, pos + vec}; }
			ENGINE_INLINE constexpr UniversalBlockCoord operator-(const BlockVec vec) const noexcept { return {realmId, pos - vec}; }
			constexpr bool operator==(const UniversalBlockCoord&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoord toChunk() const noexcept { return { realmId, blockToChunk(pos) }; }
			ENGINE_INLINE constexpr BlockVec toChunkIndex(const UniversalChunkCoord chunkCoord) const noexcept { return blockToChunkIndex(pos, chunkCoord.pos); }
	};

	ENGINE_INLINE constexpr UniversalBlockCoord UniversalChunkCoord::toBlock() const noexcept { return { realmId, chunkToBlock(pos) }; }
	ENGINE_INLINE constexpr UniversalBlockCoordX UniversalChunkCoordX::toBlock() const noexcept { return { realmId, chunkToBlock({pos, 0}).x }; }
	ENGINE_INLINE constexpr UniversalChunkCoord UniversalRegionCoord::toChunk() const noexcept { return { realmId, regionToChunk(pos) }; }
	ENGINE_INLINE constexpr UniversalChunkCoordX UniversalRegionCoordX::toChunk() const noexcept { return { realmId, regionToChunk({pos, 0}).x }; }
}


template<>
struct Engine::Hash<Game::UniversalRegionCoord> {
	[[nodiscard]] ENGINE_INLINE size_t operator()(const Game::UniversalRegionCoord& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

template<>
struct Engine::Hash<Game::UniversalChunkCoord> {
	[[nodiscard]] ENGINE_INLINE size_t operator()(const Game::UniversalChunkCoord& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

template<>
struct Engine::Hash<Game::UniversalBlockCoord> {
	[[nodiscard]] ENGINE_INLINE size_t operator()(const Game::UniversalBlockCoord& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

template<>
struct Engine::Hash<Game::UniversalRegionCoordX> {
	[[nodiscard]] ENGINE_INLINE size_t operator()(const Game::UniversalRegionCoordX& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

template<>
struct Engine::Hash<Game::UniversalChunkCoordX> {
	[[nodiscard]] ENGINE_INLINE size_t operator()(const Game::UniversalChunkCoordX& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

template<>
struct Engine::Hash<Game::UniversalBlockCoordX> {
	[[nodiscard]] ENGINE_INLINE size_t operator()(const Game::UniversalBlockCoordX& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};


template<>
struct fmt::formatter<Game::UniversalRegionCoord> {
	constexpr auto parse(format_parse_context& ctx) const { return ctx.end(); }

	auto format(const Game::UniversalRegionCoord coord, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "UniversalRegionCoord({}, {}, r{})", coord.pos.x, coord.pos.y, coord.realmId);
	}
};

template<>
struct fmt::formatter<Game::UniversalChunkCoord> {
	constexpr auto parse(format_parse_context& ctx) const { return ctx.end(); }

	auto format(const Game::UniversalChunkCoord coord, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "UniversalChunkCoord({}, {}, r{})", coord.pos.x, coord.pos.y, coord.realmId);
	}
};

template<>
struct fmt::formatter<Game::UniversalBlockCoord> {
	constexpr auto parse(format_parse_context& ctx) const { return ctx.end(); }

	auto format(const Game::UniversalBlockCoord coord, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "UniversalBlockCoord({}, {}, r{})", coord.pos.x, coord.pos.y, coord.realmId);
	}
};
