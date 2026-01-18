// Game
#include <Game/Terrain/Layer/BlendedBiomeStructures.hpp>
#include <Game/Terrain/Layer/BlendedBiomeStructureInfo.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>


namespace Game::Terrain::Layer {
	void BlendedBiomeStructures::request(const Range<Partition>& chunkCoords, TestGenerator& generator) {
		//generator.requestAwait<BlendedBiomeStructureInfo>(chunkCoords);
		generator.request<BlendedBiomeStructureInfo>(chunkCoords);
		generator.awaitGeneration();


		// TODO: this doesn't quite work, you still need to get the structure info for the most
		//       expanded region. You you need to request the info for a larger area, and only then queue
		//       the bounding boxes.

		// TODO: Cache struct info.
		//std::vector<StructureInfo> structures; 
		//generator.get2<BlendedBiomeStructureInfo>(chunkCoord, structures);
		//
		//if (!structures.empty()) {
		//	auto cur = structures.cbegin();
		//	const auto end = structures.cend();
		//	if (cur != end) {
		//		BlockVec minBlock = cur->min;
		//		BlockVec maxBlock = cur->max;
		//
		//		while (++cur != end) {
		//			minBlock = glm::min(minBlock, cur->min);
		//			maxBlock = glm::max(maxBlock, cur->max);
		//		}
		//
		//		ChunkArea area = {
		//			.min = blockToChunk(minBlock),
		//			.max = blockToChunk(maxBlock) + ChunkVec{1, 1},
		//		};
		//
		//		// Ensure the underlying terrain is already generated.
		//		area.forEach([&](const ChunkVec pos){
		//			generator.request<BlendedBiomeBlock>({.realmId = chunkCoord.realmId, .pos = pos});
		//		});
		//
		//		// TODO: forward request to relevant biomes for each struct.
		//	}
		//}
	}

	void BlendedBiomeStructures::get(const Index chunkCoord, TestGenerator& generator, Terrain& terrain) const noexcept {
		// TODO: Consider using a BSP tree, quad tree, BVH, etc. some spatial structure.
		
		// TODO: How do we want to resolve conflicts? If we just go first come first serve
		//       then a spatial  structure isn't needed, you could just check when
		//       inserting. If its not FCFS then we need to keep all boxes and resolve at
		//       the end or else you might over-cull. For example say you have a large
		//       structure that later gets removed by a higher priority small structure.
		//       Since the large structure is now removed there could be other structures
		//       which are now valid since the larger structure no longer exists and
		//       doesn't collide.

		// TODO: Cull structure overlaps

		// TODO: Theoretically a structure could overlap into unloaded/unfinished chunks.
		//       Don't we need to check and then generate those chunks here? Post culling.
		//       Or add some extra buffer to the initial request based on the largest known structure. <-- Easiest, but worse perf.
		//       
		//       Maybe at this point we could issue additional generation requests to generate
		//       those needed chunks. This is probably be the best option, but we need to add a
		//       way to generate those structs up to the block stage and then not to structs.
		//       Right now we already store the stage so we just need to add a stage for structs
		//       and that should be fairly doable.

		std::vector<StructureInfo> structures;
		generator.get2<Layer::BlendedBiomeStructureInfo>(chunkCoord, structures);
		for (const auto& info : structures) {
			Engine::withTypeAt<Biomes>(info.biomeId, [&]<class Biome>(){
				// TODO: document somewhere the structure is optional.
				if constexpr (requires { typename Biome::Structure; }) {
					ENGINE_DEBUG_ONLY(const auto [_, inserted] = generatedStructures.emplace(info));
					if constexpr (ENGINE_DEBUG) {
						if (!inserted) {
							ENGINE_ERROR2("Generating duplicate structure. This is a bug. Either with the generator itself or more likely the the biome structure info layer. Biome: {}; Structure: {}", info.biomeId, info.id);
						}
					}

					generator.get2<typename Biome::Structure>(terrain, chunkCoord.realmId, info);
				}
			});
		}
	}
}
