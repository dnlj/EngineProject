// Game
#include <Game/Terrain/Layer/BiomeOcean.hpp>
#include <Game/Terrain/TestGenerator.hpp>


namespace Game::Terrain::Layer {
	Float BiomeOceanBasis::get(BIOME_BASIS_ARGS) const noexcept {
		if (blockCoord.pos.y > h2) {
			return -1;
		}

		return 1;
	}

	BlockId BiomeOceanBlock::get(BIOME_BLOCK_ARGS) const noexcept {
		auto const& shared = generator.shared<BiomeOceanSharedData>();
		auto const& simplex1 = shared.simplex1;
		auto const& simplex2 = shared.simplex2;
		auto const& simplex3 = shared.simplex3;

		auto thresh = 0.45_f;
		thresh += 0.04_f * simplex1.value(blockCoordF * 0.025_f);
		thresh += 0.02_f * simplex2.value(blockCoordF * 0.05_f);
		thresh += 0.01_f + 0.01_f * simplex3.value(blockCoordF * 0.1_f);

		if (basisInfo.weight > thresh) {
			return BlockId::Grass;
		}

		return BlockId::Gold;
	};

	void BiomeOceanStructureInfo::get(BIOME_STRUCTURE_INFO_ARGS) const noexcept {
		const auto minBlockCoord = chunkCoord.toBlock().pos;
		inserter = {.min = minBlockCoord, .max = minBlockCoord + BlockVec{1,1}, .id = 1};
	}

	void BiomeOceanStructure::get(BIOME_STRUCTURE_ARGS) const noexcept {
		const UniversalChunkCoord chunkCoord = { .realmId = realmId, .pos = blockToChunk(info.min) };
		auto& chunk = terrain.getChunkMutable(chunkCoord);
		const auto chunkIdx = blockToChunkIndex(info.min, chunkCoord.pos);
		chunk.data[chunkIdx.x][chunkIdx.y] = BlockId::Entity;

		// TODO: Populate portal with realm data/id/pos.
		//auto& ents = terrain.getEntitiesMutable(chunkCoord);
		//auto& ent = ents.emplace_back();
		//ent.pos = info.min;
		//ent.data.type = BlockEntityType::Portal;
		//ent.data.asPortal.realmId = !realmId;
		//ent.data.asPortal.blockPos = {10, 10};
	}
}
