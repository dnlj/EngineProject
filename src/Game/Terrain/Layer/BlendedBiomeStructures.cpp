// Game
#include <Game/Terrain/Layer/BlendedBiomeStructures.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>


namespace Game::Terrain::Layer {
	void BlendedBiomeStructures::request(const Partition chunkCoord, TestGenerator& generator) {
		generator.request<BlendedBiomeStructureInfo>(chunkCoord);
	}

	void BlendedBiomeStructures::get(const Index chunkCoord, TestGenerator& generator, const RealmId realmId, Terrain& terrain) const noexcept {
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
			//
			//
			//
			//
			// TODO: need a way to check if a structure has already been generated.
			//
			//
			//
			//
			//
			//
			//
			Engine::withTypeAt<Biomes>(info.biomeId, [&]<class Biome>(){
				// TODO: document somewhere the structure is optional.
				if constexpr (requires { typename Biome::Structure; }) {
					generator.get2<typename Biome::Structure>(terrain, realmId, info);
				}
			});
		}
	}
}
