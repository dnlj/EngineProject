#pragma once

#include <Game/Terrain/Generator.hpp>


namespace Game::Terrain {
	template<class Self, class Layers, class SharedData>
	void Generator<Self, Layers, SharedData>::generate(Terrain& terrain, const Request& request) {
		// TODO: Move/redocument things in terms of layers once transition is done.
		// - Generate stages.
		//   - Stage 1, Stage 2, ..., Stage N.
		// - Generate candidate features for all biomes in the request.
		// - Cull candidate features based on point (or AABB?) and realized biome.
		// - Generate non-culled features based on resolved biome.
		// - Generate decorations.
		//   - Basically same as stages.
		//   - Provides a place to smooth/integrate features and terrain.
		//   - Moss, grass tufts, cobwebs, chests/loot, etc.
		//   - Do these things really need extra passes? Could this be done during the initial stages and feature generation?

		// TODO: avoid name conflict with arguments `request`
		const ChunkArea chunkArea = {request.minChunkCoord, request.maxChunkCoord};
		this->request<Layer::BiomeBlock>(chunkArea);
		this->request<Layer::BiomeStructures>(chunkArea);
		generateLayers();

		// Copy the generator data to the terrain.
		for (auto chunkCoord = request.minChunkCoord; chunkCoord.x < request.maxChunkCoord.x; ++chunkCoord.x) {
			for (chunkCoord.y = request.minChunkCoord.y; chunkCoord.y < request.maxChunkCoord.y; ++chunkCoord.y) {
				const auto regionCoord = chunkToRegion(chunkCoord);
				auto& region = terrain.getRegion({request.realmId, regionCoord});
				const auto regionIdx = chunkToRegionIndex(chunkCoord, regionCoord);
				auto& populated = region.populated[regionIdx.x][regionIdx.y];
				
				if (!populated) {
					// For each block in the chunk.
					// TODO: Once layers are done this should go away. Can just access the
					//       layerBiomeBlock directly.
					const auto& chunkBiomeBlock = layerBiomeBlock.get(chunkCoord);
					auto& chunk = region.chunkAt(regionIdx);
					chunk = chunkBiomeBlock;
					populated = true;
				}
			}
		}

		// TODO: Should be part of layers and/or have a populated check.
		layerBiomeStructures.get(chunkArea, self(), request.realmId, terrain);
	}
}
