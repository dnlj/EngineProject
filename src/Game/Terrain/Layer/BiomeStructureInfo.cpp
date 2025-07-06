// Game
#include <Game/Terrain/Layer/BiomeStructureInfo.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>


namespace Game::Terrain::Layer {
	void BiomeStructureInfo::request(const Range chunkArea, TestGenerator& generator) {
		generator.request<BiomeBlended>(chunkArea);
	}

	void BiomeStructureInfo::generate(const Range chunkArea, TestGenerator& generator) {
		// No need for caching.
		// This data is only ever used exactly once so caching is overhead.
	}

	void BiomeStructureInfo::get(const Index chunkArea, TestGenerator& generator, std::vector<StructureInfo>& structures) const noexcept {
		for (auto chunkCoord = chunkArea.min; chunkCoord.x < chunkArea.max.x; ++chunkCoord.x) {
			for (chunkCoord.y = chunkArea.min.y; chunkCoord.y < chunkArea.max.y; ++chunkCoord.y) {
				populate(chunkCoord, generator, structures);
			}
		}
	}

	void BiomeStructureInfo::populate(const ChunkVec chunkCoord, TestGenerator& generator, std::vector<StructureInfo>& structures) const noexcept {
		// TODO: Would there be any perf implications where it might be better to cache
		//       the struct info so that things are processed more layer-like (as opposed
		//       to on-the-fly, like is done here)? My gut reaction would be that since
		//       structure generation is (will be) quite random access I doubt there would
		//       be much if any gain.
		const auto& chunkBiomeBlended = generator.get<BiomeBlended>(chunkCoord);
		Engine::StaticVector<BiomeId, 4> biomes;
		
		// TODO: Is this true? Investigate and leave more details on why. Static asserts if possible.
		//       > Get a unique list of the biome in each corner of this chunk. Used for
		//       > during generation. At generation time checking each corner should yield a
		//       > unique list of all biomes in this chunk so long as the minimum biome size
		//       > is greater than or equal to the chunk size.
		{ // Should be moved into dedicated layer w/ caching if any other consumers are ever needed.
			const auto tryAdd = [&](const BlockVec chunkIndex) ENGINE_INLINE {
				const auto biomeId = maxBiomeWeight(chunkBiomeBlended.at(chunkIndex).weights).id;
				if (!std::ranges::contains(biomes, biomeId))  { biomes.push_back(biomeId); }
			};

			// We need -1 to chunkSize since these are indexes, not sizes.
			tryAdd({0, 0});
			tryAdd({0, chunkSize.y - 1});
			tryAdd({chunkSize.x - 1, 0});
			tryAdd({chunkSize.x - 1, chunkSize.y - 1});
		}

		for (const auto& biomeId : biomes) {
			const auto before = structures.size();

			Engine::withTypeAt<Biomes>(biomeId, [&]<class Biome>(){
				// TODO: remove direct layer access to height cache.
				// TODO: document somewhere the structure info is optional.
				if constexpr (requires { typename Biome::StructureInfo; }) {
					generator.get2<typename Biome::StructureInfo>(chunkCoord, generator.layerBiomeHeight.cache, std::back_inserter(structures));
				}
			});

			// TODO: could this be done with a custom back_inserter instead of an extra loop after the fact?
			const auto after = structures.size();
			if (after != before) {
				for (auto i = before; i < after; ++i) {
					auto& info = structures[i];
					ENGINE_DEBUG_ASSERT(info.min.x <= info.max.x);
					ENGINE_DEBUG_ASSERT(info.min.y <= info.max.y);
					ENGINE_DEBUG_ASSERT(info.biomeId == BiomeId{}, "The structure info biome id should not be populated by the biomes themselves.");
					info.biomeId = biomeId;
				}
			}
		}
	}
}
