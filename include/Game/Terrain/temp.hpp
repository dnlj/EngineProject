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
#include <Game/Terrain/BlockSpanCache.hpp>

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
	const ::Game::Terrain::BlockSpanCache<BlockUnit>& h2Cache, \
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
}

// TODO: make all cache/store types uncopyable. These should be accessed by ref.
// TODO: split out
namespace Game::Terrain {
	class StructureInfo {
		public:
			/** Structure min bounds. Inclusive. */
			BlockVec min;

			/** Structure max bounds. Exclusive. */
			BlockVec max;

			/** A id to identify this structure. Defined by each biome. */
			uint32 id;

			// We would like this to be private so it cant accidentally be modified during
			// biome structure generation, but that causes constructor issues since we use
			// a back_inserter which doesn't play nice with conversion. More hastle than
			// is worth right now.
			/** The biome this structure is in. */
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
			// TODO: Consider using some type of sparse structure/partitioning
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
				// TODO: Again, could benefit from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->chunkAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			Chunk& getChunkMutable(const UniversalChunkCoord chunkCoord) noexcept {
				// TODO: Again, could benefit from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->chunkAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			const ChunkEntities& getEntities(const UniversalChunkCoord chunkCoord) const noexcept {
				// TODO: Again, could benefit from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end());
				return found->second->entitiesAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			ChunkEntities& getEntitiesMutable(const UniversalChunkCoord chunkCoord) noexcept {
				// TODO: Again, could benefit from region caching. See notes in isChunkLoaded.
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
}
