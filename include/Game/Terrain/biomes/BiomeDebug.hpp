#include <Game/Terrain/biome.hpp>

namespace Game::Terrain {
	template<uint64 Seed, Float HAmp, Float HFeatScale, Float BScale, Float BOff, auto BTrans = [](auto b){ return b; }>
	struct BiomeDebugBase : public SimpleBiome  {
		Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
		Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
		Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};

		Float getBasisStrength(TERRAIN_GET_BASIS_STRENGTH_ARGS) {
			// These need to be tuned based on biome scales blend dist or else you can get odd clipping type issues.
			return 0.2_f * simplex1.value(glm::vec2{blockCoord} * 0.003_f)
				 + 0.2_f * simplex2.value(glm::vec2{blockCoord} * 0.010_f)
				 + 0.1_f * simplex3.value(glm::vec2{blockCoord} * 0.100_f)
				 + 0.5_f;
		}

		Float getBasis(TERRAIN_GET_BASIS_ARGS) {
			const auto h1 = h0 + HAmp * simplex1.value(blockCoord.x * HFeatScale, 0); // TODO: 1d simplex
			if (blockCoord.y > h1) { return outGrad(h1, blockCoord.y, 1.0_f / 8.0_f); }

			return
				+ inGrad(h1, blockCoord.y, 1.0_f / 200.0_f)
				+ BTrans(simplex1.value(glm::vec2{blockCoord} * BScale)) - BOff;
		}
	};

	struct BiomeDebugOne : public BiomeDebugBase<0xF7F7'F7F7'F7F7'1111, 15.0_f, 0.02_f, 0.03_f, 0.15_f, &std::fabsf> {
		STAGE_DEF;
		STAGE(1) { return BlockId::Debug; }
	};
	
	struct BiomeDebugTwo : public BiomeDebugBase<0xF7F7'F7F7'F7F7'2222, 30.0_f, 0.04_f, 0.06_f, 0.75_f, &std::fabsf> {
		STAGE_DEF;
		STAGE(1) { return BlockId::Debug2; }
	};
	
	struct BiomeDebugThree : public BiomeDebugBase<0xF7F7'F7F7'F7F7'3333, 60.0_f, 0.06_f, 0.12_f, 0.0_f> {
		STAGE_DEF;
		STAGE(1) { return BlockId::Debug3; }
	};
}
