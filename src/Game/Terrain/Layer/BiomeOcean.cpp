// Game
#include <Game/Terrain/Layer/BiomeOcean.hpp>


namespace Game::Terrain::Layer {
	Float BiomeOceanBasis::get(BIOME_BASIS_ARGS) const noexcept {
		if (blockCoord.y > h2) {
			return -1;
		}

		return 1;
	}
	BlockId BiomeOceanBlock::get(BIOME_BLOCK_ARGS) const noexcept {
		auto thresh = 0.45_f;

		// TODO: Shouldn't these use simplex 1/2/3 instead of 1/1/2?
		thresh += 0.04_f * simplex1.value(FVec2{blockCoord} * 0.025_f);
		thresh += 0.02_f * simplex1.value(FVec2{blockCoord} * 0.05_f);
		thresh += 0.01_f + 0.01_f * simplex2.value(FVec2{blockCoord} * 0.1_f);

		if (basisInfo.weight > thresh) {
			return BlockId::Grass;
		}

		return BlockId::Gold;
	};
}
