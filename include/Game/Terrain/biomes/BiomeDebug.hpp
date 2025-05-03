#include <Game/Terrain/biome.hpp>

// TODO: look into using noise derivatives, not sure how impactful it will be in 2D:
//       https://iquilezles.org/articles/morenoise/

namespace Game::Terrain { namespace {
	Float heightGrad(BlockUnit hb, BlockUnit yb, BlockUnit fadeb) {
		// Fade from one to -1 over a distance of fade for y values above h. Useful for
		// debugging blending. Something like:
		//   Float getBasis(TERRAIN_GET_BASIS_ARGS) { return heightGrad(h0 + 8, blockCoord.y, 16); }
		// 
		// Note that you may will still see sharp transitions if the heiht discrepency is
		// larger than the fade distances. For a truely smooth transition we need to scale
		// the fade distance with the biome strength or always use a fade distances that
		// is larger than the maximum discrepency possible. It should also be noted that
		// large fade distances are undesirable because the large the fade distance the
		// more floating islands occur.
		//
		// Even if we add a dynamic fade distance based on biome weight we still are
		// limited by at most a discrepency of biomeBlendDist since that is the maximum
		// distance of which we blend. As such we should aim to always have the edge of
		// biomes roughly at h0. The between blending (biome strength) and this fade
		// distance we can still deal with some differences, but it is limited.

		Float h = static_cast<Float>(hb);
		Float y = static_cast<Float>(yb);
		Float fade = static_cast<Float>(fadeb);
		// `2 / dist` instead of `1 / dist` since we are going [-1, 1] instead of [-1, 0] so the distance is doubled.
		if (y > h) { return std::max(-1_f, 1_f + (h - y) * (2_f / fade)); }
		return 1_f;
	}
}}

namespace Game::Terrain {
	template<uint64 Seed, Float HAmp, Float HFeatScale, Float BScale, Float BOff, auto BTrans = [](auto b){ return b; }>
	struct BiomeDebugBase : public SimpleBiome  {
		Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
		Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
		Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};
	};

	struct BiomeDebugOne : public BiomeDebugBase<0xF7F7'F7F7'F7F7'1111, 15.0_f, 0.01_f, 0.03_f, 0.15_f, &std::fabsf> {
		STAGE_DEF;
		STAGE(1) { return BlockId::Debug; }
	};
	
	struct BiomeDebugTwo : public BiomeDebugBase<0xF7F7'F7F7'F7F7'2222, 30.0_f, 0.02_f, 0.06_f, 0.75_f, &std::fabsf> {
		STAGE_DEF;
		STAGE(1) { return BlockId::Debug2; }
	};
	
	struct BiomeDebugThree : public BiomeDebugBase<0xF7F7'F7F7'F7F7'3333, 60.0_f, 0.04_f, 0.12_f, 0.0_f> {
		STAGE_DEF;
		STAGE(1) { return BlockId::Debug3; }
	};

	// Mountains would actually be okay to spawn at multiple scales so long as they are
	// adjacent since they just end up looking like peaks.
	struct BiomeDebugMountain : public BiomeDebugBase<0xF7F7'F7F7'F7F7'4444, 60.0_f, 0.04_f, 0.12_f, 0.0_f> {
		STAGE_DEF;
		STAGE(1) { return BlockId::Debug4; }
	};

	struct BiomeDebugOcean : public BiomeDebugBase<0xF7F7'F7F7'F7F7'5555, 60.0_f, 0.04_f, 0.12_f, 0.0_f> {
		STAGE_DEF;
		STAGE(1) {
			auto thresh = 0.45_f;
			thresh += 0.04_f * simplex1.value(FVec2{blockCoord} * 0.025_f);
			thresh += 0.02_f * simplex1.value(FVec2{blockCoord} * 0.05_f);
			thresh += 0.01_f + 0.01_f * simplex2.value(FVec2{blockCoord} * 0.1_f);

			if (basisInfo.weight > thresh) {
				return BlockId::Grass;
			}

			return BlockId::Gold;
		}
	};
}
