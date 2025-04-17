#pragma once

// Game
#include <Game/BlockEntityData.hpp>
#include <Game/BlockMeta.hpp>
#include <Game/MapChunk.hpp> // TODO: Replace/rename/update MapChunk.
#include <Game/Terrain/terrain.hpp>
#include <Game/universal.hpp>
#include <Game/Terrain/TestGenerator.hpp>

// Engine
#include <Engine/Array.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Math/math.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/StaticVector.hpp>


namespace Game::Terrain {
	// TODO: Remove, use/combine with BlockSpanCache.
	class HeightCache {
		public:
			constexpr static BlockUnit invalid = std::numeric_limits<BlockUnit>::max();

		private:
			BlockUnit minBlock = 0; // Inclusive
			BlockUnit maxBlock = 0; // Exclusive
			std::vector<BlockUnit> heights;

		public:
			void reset(BlockUnit minBlockX, BlockUnit maxBlockX) {
				minBlock = minBlockX;
				maxBlock = maxBlockX;
				heights.clear();
				heights.resize(maxBlock - minBlock, invalid);
			}

			ENGINE_INLINE_REL [[nodiscard]] BlockUnit& get(const BlockUnit blockCoord) noexcept {
				debugBoundsCheck(blockCoord);
				return heights[blockCoord - minBlock];
			}

			ENGINE_INLINE_REL [[nodiscard]] BlockUnit get(const BlockUnit blockCoord) const noexcept {
				debugBoundsCheck(blockCoord);
				ENGINE_DEBUG_ASSERT(invalid != heights[blockCoord - minBlock], "Uninitialized height value at ", blockCoord);
				return heights[blockCoord - minBlock];
			}

			ENGINE_INLINE [[nodiscard]] BlockUnit getMinBlock() const noexcept { return minBlock; }
			ENGINE_INLINE [[nodiscard]] BlockUnit getMaxBlock() const noexcept { return maxBlock; }

		private:
			ENGINE_INLINE_REL void debugBoundsCheck(const BlockUnit blockCoord) const noexcept {
				ENGINE_DEBUG_ASSERT(minBlock <= blockCoord, "Height cache block coordinate is out of bounds.");
				ENGINE_DEBUG_ASSERT(blockCoord < maxBlock, "Height cache block coordinate is out of bounds.");
			}
	};
}

// TODO: split out
namespace Game::Terrain::Layer {
	// TODO: incorperate this at the terrain level to verify the correct requests are made and avoid cycles.
	template<class...>
	class DependsOn{};

	// TODO: Make a generic Area<T> with iterator based access. Iterator based access
	//       would be useful in most biome layers to replace manual bounded for loops.
	class ChunkArea {
		public:
			ChunkVec min; // Inclusive
			ChunkVec max; // Exclusive
	};
	
	class ChunkSpanX {
		public:
			BlockUnit min; // Inclusive
			BlockUnit max; // Exclusive
	};

	// TODO: make all cache/store types uncopyable. These should be accessed by ref.
	// TODO: These cache/store/span/area types probably should probably be in just Game::Terrain not Game::Terrain::Layer.
	
	template<class T>
	class ChunkStore {
		private:
			T store[chunkSize.x][chunkSize.y]{};

		public:
			ChunkStore() = default;
			ChunkStore(ChunkStore&&) = default;
			ChunkStore(const ChunkStore&) = delete;

			ENGINE_INLINE_REL T& at(ChunkVec chunkIndex) noexcept {
				ENGINE_DEBUG_ASSERT((chunkIndex.x >= 0) && (chunkIndex.x < chunkSize.x), "Attempting to access block outside of ChunkStore.");
				ENGINE_DEBUG_ASSERT((chunkIndex.y >= 0) && (chunkIndex.y < chunkSize.y), "Attempting to access block outside of ChunkStore.");
				return store[chunkIndex.x][chunkIndex.y];
			}

			ENGINE_INLINE_REL const T& at(ChunkVec chunkIndex) const noexcept {
				return const_cast<ChunkStore*>(this)->at(chunkIndex);
			}
	};

	// IsSpecialized allows for specialization while still inheriting from the 
	template<class T>
	class RegionStore {
		private:
			T store[regionSize.x][regionSize.y]{};
			bool populated[regionSize.x][regionSize.y]{};

		public:
			RegionStore() = default;
			RegionStore(RegionStore&&) = default;
			RegionStore(const RegionStore&) = delete;

			ENGINE_INLINE_REL bool isPopulated(RegionVec regionIndex) const noexcept {
				ENGINE_DEBUG_ASSERT((regionIndex.x >= 0) && (regionIndex.x < regionSize.x), "Attempting to access chunk outside of RegionStore.");
				ENGINE_DEBUG_ASSERT((regionIndex.y >= 0) && (regionIndex.y < regionSize.y), "Attempting to access chunk outside of RegionStore.");
				return populated[regionIndex.x][regionIndex.y];
			}

			ENGINE_INLINE_REL void setPopulated(RegionVec regionIndex) noexcept {
				ENGINE_DEBUG_ASSERT((regionIndex.x >= 0) && (regionIndex.x < regionSize.x), "Attempting to access chunk outside of RegionStore.");
				ENGINE_DEBUG_ASSERT((regionIndex.y >= 0) && (regionIndex.y < regionSize.y), "Attempting to access chunk outside of RegionStore.");
				populated[regionIndex.x][regionIndex.y] = true;
			}

			ENGINE_INLINE_REL T& at(RegionVec regionIndex) noexcept {
				ENGINE_DEBUG_ASSERT((regionIndex.x >= 0) && (regionIndex.x < regionSize.x), "Attempting to access chunk outside of RegionStore.");
				ENGINE_DEBUG_ASSERT((regionIndex.y >= 0) && (regionIndex.y < regionSize.y), "Attempting to access chunk outside of RegionStore.");
				return store[regionIndex.x][regionIndex.y];
			}

			ENGINE_INLINE_REL const T& at(RegionVec regionIndex) const noexcept {
				ENGINE_DEBUG_ASSERT(isPopulated(regionIndex), "Attempting to access unpopulated chunk.");
				return const_cast<RegionStore*>(this)->at(regionIndex);
			}
	};

	/**
	 * Store chunk data grouped at the region level.
	 */
	template<class ChunkData>
	class RegionCache {
		private:
			using Store = RegionStore<ChunkData>;
			Engine::FlatHashMap<RegionVec, std::unique_ptr<Store>> regions;

		public:
			RegionCache() = default;
			RegionCache(RegionCache&&) = default;
			RegionCache(const RegionCache&) = delete;

			ENGINE_INLINE Store& at(RegionVec regionCoord) noexcept {
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end(), "Attempting to access region outside of RegionCache.");
				return *found->second;
			}

			ENGINE_INLINE const Store& at(RegionVec regionCoord) const noexcept {
				return const_cast<RegionCache*>(this)->at(regionCoord);
			}

			ENGINE_INLINE void reserve(RegionVec regionCoord) noexcept {
				const auto found = regions.find(regionCoord);
				if (found == regions.end()) {
					regions.try_emplace(regionCoord, std::make_unique<Store>());
				}
			}

			// TODO: Function sig concept
			ENGINE_INLINE void forEachChunk(ChunkArea area, auto&& func) {
				// TODO: Could be a bit more effecient by dividing into regions first. Then you
				//       just iterate over the regions+indexes directly. instead of doing
				//       chunkToRegion + chunkToRegionIndex for every chunk in the Range. It would
				//       also be more efficient because we would only need to do chunkToRegion
				//       once and then can use offsets instead of per chunk.
				for (auto chunkCoord = area.min; chunkCoord.x < area.max.x; ++chunkCoord.x) {
					for (chunkCoord.y = area.min.y; chunkCoord.y < area.max.y; ++chunkCoord.y) {
						const auto regionCoord = chunkToRegion(chunkCoord);
						reserve(regionCoord);

						auto& regionStore = at(regionCoord);
						const auto regionIndex = chunkToRegionIndex(chunkCoord, regionCoord);

						if (!regionStore.isPopulated(regionIndex)) {
							regionStore.setPopulated(regionIndex);
							auto& chunkData = regionStore.at(regionIndex);
							func(chunkCoord, chunkData);
						}
					}
				}
			}
	};

	/**
	 * Caches block data grouped at the chunk level.
	 */
	template<class BlockData>
	class ChunkCache : public RegionCache<ChunkStore<BlockData>> {};


	// TODO: Doc, caches value for every block in a span. In increments of chunks. This is
	//       done per chunk because it is easier to interop with other caches and layers that
	//       typically work per-chunk and avoids conversion/off-by-one errors.
	template<class T>
	class BlockSpanCache {
		public:
			/**
			 * Iterates each block in the given span.
			 */
			class Iterator {
				private:
					friend class BlockSpanCache;
					BlockSpanCache& cache;
					ChunkUnit chunkCoord;
					const ChunkUnit chunkCoordMax;
					BlockUnit chunkIndex = 0;
					BlockUnit blockCoord = chunkCoord * blocksPerChunk;

					Iterator(
						BlockSpanCache& cache,
						ChunkSpanX area)
						: cache{cache}
						, chunkCoord{area.min}
						, chunkCoordMax{area.max}
					{}

				public:
					ENGINE_INLINE Iterator& operator++() noexcept {
						ENGINE_DEBUG_ASSERT(chunkCoord != chunkCoordMax);
						++blockCoord;
						++chunkIndex;
						if (chunkIndex == chunkSize.x) {
							++chunkCoord;
							chunkIndex = 0;
						}
						return *this;
					}

					ENGINE_INLINE T& operator*() noexcept { return cache.at(blockCoord); }
					ENGINE_INLINE operator bool() const noexcept { return chunkCoord != chunkCoordMax; }
					ENGINE_INLINE ChunkUnit getChunkCoord() const noexcept { return chunkCoord; }
					ENGINE_INLINE BlockUnit getChunkIndex() const noexcept { return chunkIndex; }
					ENGINE_INLINE BlockUnit getBlockCoord() const noexcept { return blockCoord; }
			};

		public: // TODO: private, currently public for easy TerrainPreview compat during transition.
			static_assert(std::same_as<T, BlockUnit>, "BlockUnit is currently the only supported type due to the use of HeightCache.");
			HeightCache cache; // TODO: remove HeightCache, integrate into BlockSpanCache.

		public:
			BlockSpanCache() = default;
			BlockSpanCache(BlockSpanCache&&) = default;
			BlockSpanCache(const BlockSpanCache&) = delete;

			ENGINE_INLINE Iterator walk(ChunkSpanX area) noexcept {
				return Iterator{*this, area};
			}

			ENGINE_INLINE T& at(const BlockUnit x) noexcept {
				return cache.get(x);
			}

			ENGINE_INLINE T at(const BlockUnit x) const noexcept {
				static_assert(sizeof(T) < 4*sizeof(size_t), "Returning large type by value, this is likely not intended.");
				return cache.get(x);
			}

			ENGINE_INLINE void reserve(const ChunkSpanX area) noexcept {
				// TODO: Reset isn't quite right here. See comments in WorldBaseHeight and
				//       BiomeHeight ::request function.
				cache.reset(area.min * blocksPerChunk, area.max * blocksPerChunk);
			}
			
			// TODO: Function sig concept
			ENGINE_INLINE void forEachBlock(const ChunkSpanX area, auto&& func) {
				const auto min = area.min * blocksPerChunk;
				const auto max = area.max * blocksPerChunk;
				for (auto x = min; x < max; ++x) {
					func(x, at(x));
				}
			}
			
			// TODO: Function sig concept
			ENGINE_INLINE void forEachChunk(const ChunkSpanX area, auto&& func) {
				const auto min = area.min;
				const auto max = area.max;
				for (auto x = min; x < max; ++x) {
					func(x, at(x));
				}
			}
	};
}

template<>
struct fmt::formatter<Game::Terrain::Layer::ChunkArea> {
	constexpr auto parse(format_parse_context& ctx) const {
		return ctx.end();
	}

	auto format(const Game::Terrain::Layer::ChunkArea area, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "ChunkArea({}, {}) = BlockArea({}, {})",
			area.min,
			area.max,
			area.min * Game::blocksPerChunk,
			area.max * Game::blocksPerChunk
		);
	}
};

template<>
struct fmt::formatter<Game::Terrain::Layer::ChunkSpanX> {
	constexpr auto parse(format_parse_context& ctx) const {
		return ctx.end();
	}

	auto format(const Game::Terrain::Layer::ChunkSpanX area, format_context& ctx) const {
		return fmt::format_to(ctx.out(), "ChunkSpanX({}, {}) = BlockArea({}, {})",
			area.min,
			area.max,
			area.min * Game::blocksPerChunk,
			area.max * Game::blocksPerChunk
		);
	}
};

// TODO: split out
namespace Game::Terrain {
	using Biomes = Meta::TypeSet::TypeSet<
		// TODO: these should be class not struct...
		struct BiomeOne,
		struct BiomeDebugOne,
		struct BiomeDebugTwo,
		struct BiomeDebugThree,
		struct BiomeDebugMountain,
		struct BiomeDebugOcean
	>;
	constexpr inline auto biomeCount = Biomes::size; // TODO: define in terms of TestGenerator/param

	class BiomeScaleMeta {
		public:
			BlockUnit size;

			/** Percentage frequency out of [0, 255]. */
			uint8 freq;
	};
	
	// Each scale must be divisible by the previous depth. If they aren't then you will
	// get small "remainder" biomes around the edges of large ones which isn't a technical
	// issue, but it looks very strange to have a very small biome.
	//
	// We want to use divisions by three so that each biome can potentially spawn at surface
	// level. If we use two then only the first depth will be at surface level and all others
	// will be above or below it. This is due to the division by two for the biomeScaleOffset.
	// We need division by two because we want biomes evenly centered on y=0.
	constexpr inline BiomeScaleMeta biomeScaleSmall = { .size = 600, .freq = 0 };// TODO: use pow 2? 512?
	constexpr inline BiomeScaleMeta biomeScaleMed = { .size = biomeScaleSmall.size * 3, .freq = 20 };
	constexpr inline BiomeScaleMeta biomeScaleLarge = { .size = biomeScaleMed.size * 3, .freq = 20 };

	constexpr inline int32 biomeBlendDist = 200;
	constexpr inline int32 biomeBlendDist2 = biomeBlendDist / 2;
	static_assert(2 * biomeBlendDist2 == biomeBlendDist);
	static_assert(biomeBlendDist <= biomeScaleSmall.size / 2, "Biome blend distances cannot be larger than half the minimum biome size.");

	// The main idea is that we want the biomes to be centered on {0,0} so that surface
	// level (Y) is in the middle of a biome instead of right on the edge. Same for X, we
	// want the center of the world to be in the middle instead of on the edge.
	//
	// This is why its important that each biome scale is a 3x multiple of the previous.
	// That makes it so that when you divide by two all biomes are centered regardless of scale.
	constexpr static BlockVec biomeScaleOffset = {
		biomeScaleLarge.size / 2,
		// TODO: recalc the +200 part, that is from the old map gen
		// Experimentally terrain surface is around 100-300
		biomeScaleLarge.size / 2,// + 200.0
	};

	//enum class BiomeType : uint8 {
	//	Default,
	//	Forest,
	//	Jungle,
	//	Taiga,
	//	Desert,
	//	Savanna,
	//	Ocean,
	//	_count,
	//};
	//ENGINE_BUILD_DECAY_ENUM(BiomeType);

	using BiomeId = uint8;

	class StructureInfo {
		public:
			//StructureInfo(BlockVec min, BlockVec max, uint32 id)
			//	: min{min}, max{max}, id{id} {
			//}

			/** Structure min bounds. Inclusive. */
			BlockVec min;

			/** Structure max bounds. Exclusive. */
			BlockVec max;

			/** A id to identify this structure. Defined by each biome. */
			uint32 id;

		//protected:
			bool generate = true;
			BiomeId biomeId = {};
	};

	class BiomeRawInfo2 {
		public:
			BiomeId id;
	
			/**
			 * The coordinate in the biome map for this biome.
			 * This is not a block coordinate. This is the coordinate where this biome
			 * would be located in a theoretical grid if all biomes where the same size.
			 */
			BlockVec smallCell;
	
			/** The remaining block position within the size of a small cell. */
			BlockVec smallRem;

			// TODO: doc
			BlockVec biomeCell;
			BlockVec biomeRem;

			/**
			 * The width/height of the biome cell. This will be one of a few fixed values.
			 * @see biomeScaleSmall, biomeScaleMed, biomeScaleLarge
			 */
			BlockUnit size;
	};

	class BiomeWeight {
		public:
			BiomeId id;
			float32 weight;
	};

	using BiomeWeights = Engine::StaticVector<BiomeWeight, 4>;

	class BiomeBlend {
		public:
			BiomeRawInfo2 info;
			BiomeWeights weights;
			BiomeWeights rawWeights;
	};

	inline void normalizeBiomeWeights(BiomeWeights& weights) {
		const auto total = std::reduce(weights.cbegin(), weights.cend(), 0.0f, [](Float accum, const auto& value){ return accum + value.weight; });
		const auto normF = 1.0f / total;
		for (auto& w : weights) { w.weight *= normF; }
	}

	[[nodiscard]] ENGINE_INLINE inline BiomeWeight maxBiomeWeight(const BiomeWeights& weights) {
		return *std::ranges::max_element(weights, {}, &BiomeWeight::weight);
	}

	// TODO: Once everything is converted to layers i think this class can just go away.
	//       We just need a float for basis and other values can be pulled from other systems.
	//       Unless there is value in caching things like weight here.
	class BasisInfo {
		public:
			BiomeId id;
			Float weight;
			Float basis;
	};

	using ChunkEntities = std::vector<BlockEntityDesc>;
	// TODO: Replace/rename/update MapChunk.
	using Chunk = MapChunk;
	// TODO: getters with debug bounds checking.
	//class Chunk {
	//	public:
	//		// TODO: combine id/data arrays? Depends on how much data we have once everything is done.
	//		BlockId data[chunkSize.x][chunkSize.y]{};
	//};

	// TODO: getters with debug bounds checking.
	class Region {
		public:
			/** Chunk data for each chunk in the region. */
			Chunk chunks[regionSize.x][regionSize.y]{};

			/** The current stage of each chunk. Stage zero is uninitialized. */
			StageId stages[regionSize.x][regionSize.y]{};

			BiomeId biomes[biomesPerRegion.x][biomesPerRegion.y]{};

			// TODO: These are currently never used/generated. Waiting on MapSystem integration.
			//
			// TODO: What about non-block entities? Currently no use case so no point in supporting.
			// 
			// TODO: This (BlockEntityDesc) is a hold over from the old map generator. Can
			//       probably rework to be a good bit simpler. No point in doing that until we
			//       solve disk serialization. Can probably reuse some of that here.
			// 
			// TODO: Consider using some type of sparse structure/partitonaing
			//       here(BSP/QuadTree/etc.). Most chunks won't have entities. And those
			//       that do will probably only have a few. Could do one sparse per
			//       region or sparse per chunks and dense entities.
			// 
			// Entities per chunk.
			ChunkEntities entities[regionSize.x][regionSize.y]{};

			ENGINE_INLINE constexpr Chunk& chunkAt(RegionIdx regionIdx) noexcept { return chunks[regionIdx.x][regionIdx.y]; }
			ENGINE_INLINE constexpr const Chunk& chunkAt(RegionIdx regionIdx) const noexcept { return chunks[regionIdx.x][regionIdx.y]; }

			ENGINE_INLINE constexpr StageId stageAt(RegionIdx regionIdx) const noexcept { return stages[regionIdx.x][regionIdx.y]; }

			ENGINE_INLINE constexpr ChunkEntities& entitiesAt(RegionIdx regionIdx) noexcept { return entities[regionIdx.x][regionIdx.y]; }
			ENGINE_INLINE constexpr const ChunkEntities& entitiesAt(RegionIdx regionIdx) const noexcept { return entities[regionIdx.x][regionIdx.y]; }

			//ENGINE_INLINE constexpr BiomeId& biomeAt(RegionBiomeIdx regionBiomeIdx) noexcept { return biomes[regionBiomeIdx.x][regionBiomeIdx.y]; }
			//ENGINE_INLINE constexpr BiomeId biomeAt(RegionBiomeIdx regionBiomeIdx) const noexcept { return biomes[regionBiomeIdx.x][regionBiomeIdx.y]; }

			/**
			 * Get a unique list of the biome in each corner of this chunk. Used for
			 * during generation. At generation time checking each corner should yield a
			 * unique list of all biomes in this chunk so long as the minimum biome size
			 * is greater than or equal to the chunk size.
			 */
			Engine::StaticVector<BiomeId, 4> getUniqueBiomesApprox(const RegionIdx regionIdx) const noexcept {
				Engine::StaticVector<BiomeId, 4> results;
			
				const auto maybeAdd = [&](BiomeId id) ENGINE_INLINE {
					if (!std::ranges::contains(results, id))  { results.push_back(id); }
				};

				const auto regionBiomeIdx = regionIdxToRegionBiomeIdx(regionIdx);
				constexpr static auto off = biomesPerChunk - 1;
				maybeAdd(biomes[regionBiomeIdx.x][regionBiomeIdx.y]);
				maybeAdd(biomes[regionBiomeIdx.x][regionBiomeIdx.y + off]);
				maybeAdd(biomes[regionBiomeIdx.x + off][regionBiomeIdx.y]);
				maybeAdd(biomes[regionBiomeIdx.x + off][regionBiomeIdx.y + off]);
			
				return results;
			}
	};

	class Terrain {
		private:
			Engine::FlatHashMap<UniversalRegionCoord, std::unique_ptr<Region>> regions;

		public:
			void eraseRegion(const UniversalRegionCoord regionCoord) noexcept {
				regions.erase(regionCoord);
			}

			Region& getRegion(const UniversalRegionCoord regionCoord) noexcept {
				const auto found = regions.find(regionCoord);
				if (found == regions.end()) {
					return *regions.try_emplace(regionCoord, std::make_unique<Region>()).first->second;
				}
				return *found->second;
			}

			bool isRegionLoaded(const UniversalRegionCoord regionCoord) const noexcept {
				const auto found = regions.find(regionCoord);
				return found != regions.end();
			}

			// TODO: Should have a way to combine isChunkLoaded with getChunk. In most
			//       (all?) places we have a patterna like:
			//           if (!terrain.isChunkLoaded(pos)) { return; }
			//           auto& chunk = terrain.getChunk(pos);
			//           // Do something with chunk.
			//       Really this applies to all functions here, they are all used in
			//       conjunction with each other and all call the same few functions
			//       redundantly.

			// TODO: rename/update to (stage check): bool isChunkFinalized(const UniversalChunkCoord chunkCoord) const {
			bool isChunkLoaded(const UniversalChunkCoord chunkCoord) const noexcept {
				// TODO: Cache last region checked? Since we are always checking
				//       sequential chunks its very likely that all checks will be for the same
				//       region. May not be worth. Would need to profile.

				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				if (found == regions.end()) {
					return false;
				}

				// TODO: What does loaded really mean. Just because we aren't on stage
				// zero doesn't mean that the chunk is final. Evaluate the logic using
				// this to ensure it makes sense. I think we usually really want this to
				// be on the final stage, not just any non-zero stage.
				//
				// We could define a convention where final stage always ==
				// StageId::max(). Then the terrain doesn't need to know what the final
				// stage is.
				return found->second->stageAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			Chunk const& getChunk(const UniversalChunkCoord chunkCoord) const noexcept {
				// TODO: Again, could benefic from region caching. See notes in isChunkLoaded.
				auto const regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->chunkAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			Chunk& getChunkMutable(const UniversalChunkCoord chunkCoord) noexcept {
				// TODO: Again, could benefic from region caching. See notes in isChunkLoaded.
				auto const regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->chunkAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			const ChunkEntities& getEntities(const UniversalChunkCoord chunkCoord) const noexcept {
				// TODO: Again, could benefic from region caching. See notes in isChunkLoaded.
				auto const regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->entitiesAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			ChunkEntities& getEntitiesMutable(const UniversalChunkCoord chunkCoord) noexcept {
				// TODO: Again, could benefic from region caching. See notes in isChunkLoaded.
				auto const regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->entitiesAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			/**
			 * Ensures that space is allocated for the given chunk.
			 * This function never populates any data. If the chunk did not exist an empty
			 * chunk will be created there.
			 */
			void forceAllocateChunk(const UniversalChunkCoord chunkCoord) noexcept {
				auto const regionCoord = chunkCoord.toRegion();
				auto& region = getRegion(regionCoord);
				auto const idx = chunkToRegionIndex(chunkCoord.pos, regionCoord.pos);

				// TODO: What stage to use, see TODO in isChunkLoaded.
				auto& stage = region.stages[idx.x][idx.y];
				stage = std::max<StageId>(stage, 1);
			}
	};

	class Request {
		public:
			const ChunkVec minChunkCoord; // Inclusive
			const ChunkVec maxChunkCoord; // Inclusive
			const RealmId realmId;

		public:
			// TODO: assert in terrain generator that this won't require generating
			//       chunks, or partial chunks outside of the neighboring regions. That
			//       means that that total number of generation stages + the number of
			//       chunks loaded around a player determine the maximum request
			//       size/limit each other. Although now that we don't generate an entire
			//       region at once it is viable to increase the size of regions if
			//       needed.
			// 
			// TODO: Doc
			// - All chunks are expected to be in the neighborhood of initRegionCoord
			// - 
			//Request(const ChunkVec minChunkCoord, const ChunkVec maxChunkCoord);
	};

	template<class T, StageId Stage>
	concept HasStage = T::template hasStage<Stage>;

	template<class T, StageId Stage = 1>
	struct MaxStage : public std::conditional_t<
		HasStage<T, Stage>,
		MaxStage<T, Stage+1>,
		std::integral_constant<StageId, Stage - 1>
	> {};
}
