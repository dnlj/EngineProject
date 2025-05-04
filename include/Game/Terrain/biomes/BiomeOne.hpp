namespace Game::Terrain {
	struct BiomeOne {
		void genLandmarks(TERRAIN_GEN_LANDMARKS_ARGS) const {
			// TODO: Come up with a system for landmark ids.
			//       Currently:
			//       - info.id = 0 = tree
			//       - info.id = 1 = debug chunk marker

			// TODO: This is the least efficient way possible to do this. We need a good
			//       api to efficiently edit multiple blocks spanning multiple chunks and
			//       regions. We would need to pre split between both regions and then chunks
			//       and then do something roughly like:
			//       for region in splitRegions:
			//           for chunk in splitRegionChunks:
			//               applyEdit(chunk, editsForChunk(chunk));
			for (auto blockCoord = info.min; blockCoord.x < info.max.x; ++blockCoord.x) {
				for (blockCoord.y = info.min.y; blockCoord.y < info.max.y; ++blockCoord.y) {
					const auto chunkCoord = blockToChunk(blockCoord);
					const UniversalRegionCoord regionCoord = {realmId, chunkToRegion(chunkCoord)};
					auto& region = terrain.getRegion(regionCoord);
					const auto regionIdx = chunkToRegionIndex(chunkCoord);
					auto& chunk = region.chunks[regionIdx.x][regionIdx.y];
					const auto chunkIdx = blockToChunkIndex(blockCoord, chunkCoord);
					ENGINE_DEBUG_ASSERT(chunkIdx.x >= 0 && chunkIdx.x < chunkSize.x);
					ENGINE_DEBUG_ASSERT(chunkIdx.y >= 0 && chunkIdx.y < chunkSize.y);

					// TODO: what is this debug4 check for?
					if (chunk.data[chunkIdx.x][chunkIdx.y] != BlockId::Debug4) {
						chunk.data[chunkIdx.x][chunkIdx.y] = info.id == 0 ? BlockId::Gold : BlockId::Grass;
					}

					//ENGINE_LOG2("GEN LANDMARK: {}", chunkCoord);
				}
			}

			if (info.id == 0) {
				const auto chunkCoord = blockToChunk(info.min);
				const UniversalRegionCoord regionCoord = {realmId, chunkToRegion(chunkCoord)};
				auto& region = terrain.getRegion(regionCoord);
				const auto regionIdx = chunkToRegionIndex(chunkCoord);
				auto& ents = region.entitiesAt(regionIdx);
				auto& ent = ents.emplace_back();
				ent.pos = info.min; // TODO: center
				ent.data.type = BlockEntityType::Tree;
				ent.data.asTree.type = Engine::Noise::lcg(ent.pos.x) % 3; // TODO: random tree variant
				ent.data.asTree.size = {3, 9}; // TODO: size
			}
		}
	};
}
