#pragma once

// Game
#include <Game/BlockEntityData.hpp>
#include <Game/BlockMeta.hpp>
#include <Game/MapChunk.hpp> // TODO: Replace/rename/update MapChunk.
#include <Game/Terrain/terrain.hpp>
#include <Game/universal.hpp>
#include <Game/Terrain/ChunkArea.hpp>
#include <Game/Terrain/ChunkSpan.hpp>
#include <Game/Terrain/ChunkStore.hpp>
#include <Game/Terrain/RegionStore.hpp>
#include <Game/Terrain/RegionDataCache.hpp>
#include <Game/Terrain/ChunkDataCache.hpp>

// Engine
#include <Engine/Array.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Math/math.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/StaticVector.hpp>


#define BIOME_HEIGHT_ARGS \
	const TestGenerator& generator, \
	const ::Game::BlockUnit blockCoordX, \
	const ::Game::Terrain::Float h0, \
	const ::Game::Terrain::BiomeRawInfo2& rawInfo

#define BIOME_BASIS_STRENGTH_ARGS \
	const TestGenerator& generator, \
	const ::Game::BlockVec blockCoord

#define BIOME_BASIS_ARGS \
	const TestGenerator& generator, \
	const ::Game::BlockVec blockCoord, \
	const ::Game::BlockUnit h2

#define BIOME_BLOCK_ARGS \
	const TestGenerator& generator, \
	const ::Game::BlockVec blockCoord, \
	const ::Game::Terrain::BasisInfo& basisInfo


#define BIOME_STRUCTURE_INFO_ARGS \
	const TestGenerator& generator, \
	const ::Game::ChunkVec& chunkCoord, \
	const ::Game::Terrain::HeightCache& h2Cache, \
	std::back_insert_iterator<std::vector<::Game::Terrain::StructureInfo>> inserter


// TODO: It might be better to look into some kind of trait/tag based system
//       for landmark generation instead of having it baked in at the biome
//       level. That would allow us to say something like:
//           Boss portals for LavaGuy can spawn anywhere with a temperature > 100deg or with the tag HasLavaGuy.
//       With genLandmarks that will need to be explicitly included in every
//       relevant biome. The tag/trait based system would also be useful for generating temporary
//       entities such as mobs.
#define BIOME_STRUCTURE_ARGS \
	const TestGenerator& generator, \
	::Game::Terrain::Terrain& terrain, \
	::Game::RealmId realmId, \
	const ::Game::Terrain::StructureInfo& info

namespace Game::Terrain {
	class TestGenerator; // TODO: rm

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

	// TODO: make all cache/store types uncopyable. These should be accessed by ref.
	// TODO: These cache/store/span/area types probably should probably be in just Game::Terrain not Game::Terrain::Layer.


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

// TODO: split out
namespace Game::Terrain {
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
			bool populated[regionSize.x][regionSize.y]{};

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

			ENGINE_INLINE constexpr bool isPopulated(RegionIdx regionIdx) const noexcept { return populated[regionIdx.x][regionIdx.y]; }

			ENGINE_INLINE constexpr ChunkEntities& entitiesAt(RegionIdx regionIdx) noexcept { return entities[regionIdx.x][regionIdx.y]; }
			ENGINE_INLINE constexpr const ChunkEntities& entitiesAt(RegionIdx regionIdx) const noexcept { return entities[regionIdx.x][regionIdx.y]; }
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
			//       (all?) places we have a pattern like:
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
				return found->second->isPopulated(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			Chunk const& getChunk(const UniversalChunkCoord chunkCoord) const noexcept {
				// TODO: Again, could benefic from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->chunkAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			Chunk& getChunkMutable(const UniversalChunkCoord chunkCoord) noexcept {
				// TODO: Again, could benefic from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->chunkAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			const ChunkEntities& getEntities(const UniversalChunkCoord chunkCoord) const noexcept {
				// TODO: Again, could benefic from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->entitiesAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			ChunkEntities& getEntitiesMutable(const UniversalChunkCoord chunkCoord) noexcept {
				// TODO: Again, could benefic from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
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
				const auto regionCoord = chunkCoord.toRegion();
				auto& region = getRegion(regionCoord);
				const auto idx = chunkToRegionIndex(chunkCoord.pos, regionCoord.pos);
				region.populated[idx.x][idx.y] = true;
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
}
