#pragma once

namespace Game {
	class UniversalChunkCoord;
	class UniversalChunkCoordX;
	class UniversalChunkSpanX;
	class UniversalBlockCoord;
	class UniversalBlockCoordX;

	class UniversalRegionCoordX {
		public:
			RealmId realmId;
			RegionUnit pos;

			ENGINE_INLINE constexpr bool operator<(const UniversalRegionCoordX& right) const noexcept { return (realmId < right.realmId) && (pos < right.pos); }
			ENGINE_INLINE constexpr bool operator==(const UniversalRegionCoordX&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoordX toChunk() const noexcept;
	};

	class UniversalRegionCoord {
		public:
			RealmId realmId;
			RegionVec pos;

			ENGINE_INLINE constexpr bool operator==(const UniversalRegionCoord&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoord toChunk() const noexcept;
			ENGINE_INLINE constexpr UniversalRegionCoordX toX() const noexcept { return { realmId, pos.x }; }
	};

	class UniversalRegionSpanX {
		public:
			RealmId realmId;
			RegionUnit min;
			RegionUnit max; // Exclusive

			ENGINE_INLINE constexpr UniversalChunkSpanX toChunkSpanX() const noexcept;

			ENGINE_INLINE void forEach(auto&& func) const noexcept {
				for (RegionUnit x = min; x < max; ++x) {
					func(UniversalRegionCoordX{.realmId = realmId, .pos = x});
				}
			}
	};

	class UniversalRegionArea {
		public:
			RealmId realmId;
			RegionVec min;
			RegionVec max; // Exclusive

			ENGINE_INLINE void forEach(auto&& func) const noexcept {
				for (RegionUnit x = min.x; x < max.x; ++x) {
					for (RegionUnit y = min.y; y < max.y; ++y) {
						func(UniversalRegionCoord{.realmId = realmId, .pos = {x, y}});
					}
				}
			}

			ENGINE_INLINE UniversalRegionSpanX toSpanX() const noexcept {
				return {realmId, min.x, max.x};
			}
	};

	class UniversalChunkCoordX {
		public:
			RealmId realmId;
			ChunkUnit pos;
			
			ENGINE_INLINE constexpr bool operator==(const UniversalChunkCoordX&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoordX operator+(const UniversalChunkCoordX other) const noexcept { return {realmId, pos + other.pos}; }
			ENGINE_INLINE constexpr UniversalChunkCoordX operator-(const UniversalChunkCoordX other) const noexcept { return {realmId, pos - other.pos}; }
			ENGINE_INLINE constexpr UniversalRegionCoordX toRegion() const noexcept { return {realmId, chunkToRegion({pos, 0}).x}; }
			ENGINE_INLINE constexpr UniversalBlockCoordX toBlock() const noexcept;
	};

	class UniversalChunkCoord {
		public:
			RealmId realmId;
			ChunkVec pos;

			ENGINE_INLINE constexpr bool operator<(const UniversalChunkCoord& right) const noexcept { return (realmId < right.realmId) && (pos.x < right.pos.x) && (pos.y < right.pos.y); }
			ENGINE_INLINE constexpr bool operator==(const UniversalChunkCoord&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoord operator+(const ChunkVec vec) const noexcept { return {realmId, pos + vec}; }

			ENGINE_INLINE constexpr UniversalChunkCoordX toX() const noexcept { return { realmId, pos.x }; }
			ENGINE_INLINE constexpr UniversalRegionCoord toRegion() const noexcept { return { realmId, chunkToRegion(pos) }; }
			ENGINE_INLINE constexpr ChunkVec toRegionIndex(UniversalRegionCoord regionCoord) const noexcept { return chunkToRegionIndex(pos, regionCoord.pos); }
			ENGINE_INLINE constexpr inline UniversalBlockCoord toBlock() const noexcept;
	};

	class UniversalChunkSpanX {
		public:
			RealmId realmId;
			ChunkUnit min;
			ChunkUnit max; // Exclusive

			ENGINE_INLINE void forEach(auto&& func) const noexcept {
				for (ChunkUnit x = min; x < max; ++x) {
					func(UniversalChunkCoord{.realmId = realmId, .pos = x});
				}
			}
	};

	class UniversalChunkArea {
		public:
			RealmId realmId;
			ChunkVec min;
			ChunkVec max; // Exclusive

			ENGINE_INLINE constexpr UniversalRegionArea toRegionArea() const noexcept { return {realmId, chunkToRegion(min), chunkToRegionExclude(max)}; }

			ENGINE_INLINE void forEach(auto&& func) const noexcept {
				for (RegionUnit x = min.x; x < max.x; ++x) {
					for (RegionUnit y = min.y; y < max.y; ++y) {
						func(UniversalChunkCoord{.realmId = realmId, .pos = {x, y}});
					}
				}
			}
	};

	class UniversalBlockCoordX {
		public:
			RealmId realmId;
			BlockUnit pos;
			
			ENGINE_INLINE constexpr bool operator==(const UniversalBlockCoordX&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalBlockCoordX operator+(const BlockUnit off) const noexcept { return {realmId, pos + off}; }
			ENGINE_INLINE constexpr UniversalBlockCoordX operator-(const BlockUnit off) const noexcept { return {realmId, pos - off}; }
	};

	class UniversalBlockCoord {
		public:
			RealmId realmId;
			BlockVec pos;

			ENGINE_INLINE constexpr bool operator==(const UniversalBlockCoord&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalBlockCoord operator+(const BlockVec vec) const noexcept { return {realmId, pos + vec}; }
			ENGINE_INLINE constexpr UniversalBlockCoord operator-(const BlockVec vec) const noexcept { return {realmId, pos - vec}; }
			ENGINE_INLINE constexpr UniversalChunkCoord toChunk() const noexcept { return { realmId, blockToChunk(pos) }; }
			ENGINE_INLINE constexpr BlockVec toChunkIndex(const UniversalChunkCoord chunkCoord) const noexcept { return blockToChunkIndex(pos, chunkCoord.pos); }
	};

	class UniversalBlockArea {
		public:
			RealmId realmId;
			BlockVec min;
			BlockVec max; // Exclusive

			ENGINE_INLINE void forEach(auto&& func) const noexcept {
				for (RegionUnit x = min.x; x < max.x; ++x) {
					for (RegionUnit y = min.y; y < max.y; ++y) {
						func(UniversalBlockCoord{.realmId = realmId, .pos = {x, y}});
					}
				}
			}
	};
	
	ENGINE_INLINE constexpr UniversalBlockCoord UniversalChunkCoord::toBlock() const noexcept { return {realmId, chunkToBlock(pos)}; }
	ENGINE_INLINE constexpr UniversalBlockCoordX UniversalChunkCoordX::toBlock() const noexcept { return {realmId, chunkToBlock({pos, 0}).x}; }
	ENGINE_INLINE constexpr UniversalChunkCoord UniversalRegionCoord::toChunk() const noexcept { return {realmId, regionToChunk(pos)}; }
	ENGINE_INLINE constexpr UniversalChunkCoordX UniversalRegionCoordX::toChunk() const noexcept { return {realmId, regionToChunk({pos, 0}).x}; }
	ENGINE_INLINE constexpr UniversalChunkSpanX UniversalRegionSpanX::toChunkSpanX() const noexcept { return {realmId, regionToChunkExclude({min, 0}).x, regionToChunkExclude({max, 0}).x}; }
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

template<>
struct fmt::formatter<Game::UniversalRegionCoordX> {
	constexpr auto parse(format_parse_context& ctx) const { return ctx.end(); }

	auto format(const Game::UniversalRegionCoordX coord, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "UniversalRegionCoordX({}, r{})", coord.pos, coord.realmId);
	}
};

template<>
struct fmt::formatter<Game::UniversalChunkCoordX> {
	constexpr auto parse(format_parse_context& ctx) const { return ctx.end(); }

	auto format(const Game::UniversalChunkCoordX coord, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "UniversalChunkCoordX({}, r{})", coord.pos, coord.realmId);
	}
};

template<>
struct fmt::formatter<Game::UniversalBlockCoordX> {
	constexpr auto parse(format_parse_context& ctx) const { return ctx.end(); }

	auto format(const Game::UniversalBlockCoordX coord, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "UniversalBlockCoordX({}, r{})", coord.pos, coord.realmId);
	}
};
