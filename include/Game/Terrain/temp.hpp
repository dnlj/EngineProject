#pragma once

// Game
#include <Game/BlockEntityData.hpp>
#include <Game/BlockMeta.hpp>
#include <Game/MapChunk.hpp> // TODO: Replace/rename/update MapChunk.
#include <Game/Terrain/terrain.hpp>
#include <Game/universal.hpp>
#include <Game/Terrain/ChunkSpan.hpp>
#include <Game/Terrain/RawBiomeInfo.hpp>
#include <Game/Terrain/BiomeWeight.hpp>

// Engine
#include <Engine/Array.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Math/math.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/StaticVector.hpp>

// STD
#include <mutex>


// TODO: make all cache/store types uncopyable. These should be accessed by ref.
// TODO: split out
namespace Game::Terrain {
	template<class T>
	class Range {
		public:
			std::vector<T> range;

			ENGINE_INLINE void forEach(auto&& func) const {
				for (const auto& part : range) {
					func(part);
				}
			}
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
	// TODO: getters with debug bounds checking.
	using Chunk = MapChunk;

	enum class ChunkStage : uint8 {
		Uninitialized = 0,
		TerrainComplete,
		StructuresComplete,

		Done = StructuresComplete,
	};

	// TODO: getters with debug bounds checking.
	class Region {
		public:
			/** Chunk data for each chunk in the region. */
			Chunk chunks[regionSize.x][regionSize.y]{};

			// TODO: should we use a bitset for this? That would probably make check if all chunks populated easier.
			ChunkStage populated[regionSize.x][regionSize.y]{};

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

			ENGINE_INLINE constexpr ChunkStage getChunkStage(RegionIdx regionIdx) const noexcept { return populated[regionIdx.x][regionIdx.y]; }

			ENGINE_INLINE constexpr ChunkEntities& entitiesAt(RegionIdx regionIdx) noexcept { return entities[regionIdx.x][regionIdx.y]; }
			ENGINE_INLINE constexpr const ChunkEntities& entitiesAt(RegionIdx regionIdx) const noexcept { return entities[regionIdx.x][regionIdx.y]; }
	};

	class Terrain {
		private:
			Engine::FlatHashMap<UniversalRegionCoord, std::unique_ptr<Region>> regions;
			std::mutex mutex{};

		public:
			ENGINE_INLINE std::lock_guard<std::mutex> lock() {
				return std::lock_guard{mutex};
			}

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

			[[nodiscard]] ENGINE_INLINE bool isRegionLoaded(const UniversalRegionCoord regionCoord) const noexcept {
				ENGINE_FLATTEN {
					const auto found = regions.find(regionCoord);
					return found != regions.end();
				}
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
				return found->second->getChunkStage(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos)) == ChunkStage::Done;
			}

			Chunk const& getChunk(const UniversalChunkCoord chunkCoord) const noexcept {
				// TODO: Again, could benefit from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end(), "Attempting to access unloaded region.");
				return found->second->chunkAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			Chunk& getChunkMutable(const UniversalChunkCoord chunkCoord) noexcept {
				// TODO: Again, could benefit from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end(), "Attempting to access unloaded region.");
				return found->second->chunkAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			const ChunkEntities& getEntities(const UniversalChunkCoord chunkCoord) const noexcept {
				// TODO: Again, could benefit from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end(), "Attempting to access unloaded region.");
				return found->second->entitiesAt(chunkToRegionIndex(chunkCoord.pos, regionCoord.pos));
			}

			ChunkEntities& getEntitiesMutable(const UniversalChunkCoord chunkCoord) noexcept {
				// TODO: Again, could benefit from region caching. See notes in isChunkLoaded.
				const auto regionCoord = chunkCoord.toRegion();
				const auto found = regions.find(regionCoord);
				ENGINE_DEBUG_ASSERT(found != regions.end(), "Attempting to access unloaded region.");
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

				//
				//
				// TODO: Why is this needed. Seems like a bug. Investigate.
				//
				//
				const auto idx = chunkToRegionIndex(chunkCoord.pos, regionCoord.pos);
				region.populated[idx.x][idx.y] = ChunkStage::Done;
			}
	};
}
