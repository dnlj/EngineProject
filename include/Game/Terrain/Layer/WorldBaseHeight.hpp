#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.

namespace Game::Terrain::Layer {
	// The large world-scale height variation that persists between all biomes.
	class WorldBaseHeight : public DependsOn<> {
		public:
			using Range = RegionSpanX;
			using Partition = RegionUnit;
			using Index = RegionSpanX;

		public:
			// TODO: May be threading considerations. Maybe have an option to do
			//       independent layer/per thread/top level request?
			// TODO: Would like a way to ask it to clear cache per top level request/"batch" if possible.
			BlockSpanCache<BlockUnit> cache;

		public:
			ENGINE_INLINE void request(const Range area, TestGenerator& generator) {
				cache.reserve(area);
			}

			ENGINE_INLINE void partition(std::vector<Range>& requests, std::vector<Partition>& partitions) {
				flattenRequests(requests, partitions);
			}

			void generate(const Partition regionCoordX, TestGenerator& generator) noexcept {
				// TODO: Shouldnt this skip already generated areas?
				// TODO: use _f for Float. Move from TerrainPreview.
				// TODO: keep in mind that this is +- amplitude, and for each octave we increase the contrib;
				// TODO: tune + octaves, atm this is way to steep.

				auto& data = cache.get(regionCoordX);
				const auto baseBlockCoordX = chunkToBlock(regionToChunk({regionCoordX, 0})).x;
				for (BlockUnit blockRegionIndex = 0; blockRegionIndex < blocksPerRegion; ++blockRegionIndex) {
					const auto blockCoordX = baseBlockCoordX + blockRegionIndex;
					data[blockRegionIndex] = static_cast<BlockUnit>(500 * simplex1.value(blockCoordX * 0.00005f, 0));
				}
			}

			// TODO: remove, temp during biome span region transition.
			ENGINE_INLINE_REL [[nodiscard]] BlockUnit getOld(const BlockUnit x) const noexcept {
				return cache.at(x);
			}
			 
			ENGINE_INLINE_REL [[nodiscard]] decltype(auto) get(const Partition regionCoordX) const noexcept {
				return cache.get(regionCoordX);
			}

			ENGINE_INLINE_REL [[nodiscard]] decltype(auto) get(const Index area) const noexcept {
				return cache.walk(area);
			}
			 
			ENGINE_INLINE_REL [[nodiscard]] decltype(auto) get(const TestGenerator&, const BlockSpanX blockSpanX) const noexcept {
				return cache.walk(blockSpanX);
			}
			 
			ENGINE_INLINE_REL [[nodiscard]] decltype(auto) get(const TestGenerator&, const ChunkUnit chunkX) const noexcept {
				return cache.walk(chunkX);
			}

		private:
			// TODO: Should we have a mechanism for sharing noise generators between multiple systems?
			// TODO: should have see as constructor param.
			Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(21212)};
	};
}
