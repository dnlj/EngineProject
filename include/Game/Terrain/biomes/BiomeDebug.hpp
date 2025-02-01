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

		Float getBasisStrength(TERRAIN_GET_BASIS_STRENGTH_ARGS) {
			// These need to be tuned based on biome scales blend dist or else you can get odd clipping type issues.
			return 0.2_f * simplex1.value(FVec2{blockCoord} * 0.003_f)
				 + 0.2_f * simplex2.value(FVec2{blockCoord} * 0.010_f)
				 + 0.1_f * simplex3.value(FVec2{blockCoord} * 0.100_f)
				 + 0.5_f;
		}

		Float getHeight(TERRAIN_GET_HEIGHT_ARGS) {
			return h0 + HAmp * simplex1.value(blockCoord.x * HFeatScale, 0); // TODO: 1d simplex
		}

		Float getBasis(TERRAIN_GET_BASIS_ARGS) {

			// Note that we have limited horizontal detail because we only have fade in
			// the vertical direction. With that in mind it may be better to use fbm +
			// warp instead. That also (mostly) avoids the floating island problem.

			//const auto xWarp = 0;
			//const auto yWarp = 0;

			//const auto xWarp =
			//	+ 5.0_f * simplex1.value(FVec2{blockCoord} * 0.05f)
			//	+ 3.0_f * simplex2.value(FVec2{blockCoord} * 0.1f)
			//	+ 1.5_f * simplex3.value(FVec2{blockCoord} * 0.2f);
			//const auto yWarp =
			//	+ 5.0_f * simplex3.value(FVec2{blockCoord} * 0.05f)
			//	+ 3.0_f * simplex1.value(FVec2{blockCoord} * 0.1f)
			//	+ 1.5_f * simplex2.value(FVec2{blockCoord} * 0.2f);;
			//
			//const auto bcoord = FVec2{blockCoord} + FVec2{xWarp, yWarp};
			//
			//auto h1 = h0 + HAmp * simplex1.value(bcoord.x * HFeatScale, 0); // TODO: 1d simplex
			//h1 += 0.25_f * HAmp * simplex2.value(bcoord.x * HFeatScale * 3_f, 0); // TODO: 1d simplex
			//h1 += 0.125_f * HAmp * simplex3.value(bcoord.x * HFeatScale * 5_f, 0); // TODO: 1d simplex
			//
			//if (bcoord.y > h1) {
			//	return -1_f;
			//} else {
			//	return 1_f;
			//}

			//const auto bcoord = FVec2{blockCoord} + FVec2{xWarp, yWarp}; // TODO: rm if unused
			//auto h1 = h0 + HAmp * simplex1.value(bcoord.x * HFeatScale, 0); // TODO: 1d simplex

			// Going from 1 to 0 is effectively a max(biomeA, biomeB) since the basies are added together.
			// Going from 1 to -1 seems more correct and gives smoother transitions, but
			// result in the caves issue in mountains peaks. That seems like it should be
			// a bug elsewhere though?
			//if (bcoord.y >= h1) {
			//	// TODO: shouldn't this go from 1 to -1?
			//	// lerp from 0 to -1 over [multiplier] blocks
			//	//return std::max(-1.0_f, (h1 - bcoord.y) * (1.0f / 8.0_f));
			//	//return 1.0_f + std::max(-1.0_f, (h1 - bcoord.y) * (1.0f / 8.0_f));
			//	//return 1.0_f + 2.0_f * std::max(-1.0_f, (h1 - bcoord.y) * (1.0f / 32.0_f));
			//	//return 0.0f;
			//
			//	Float value =  std::max(-1_f, 1_f + (h1 - bcoord.y) * (2_f / 16_f));
			//	return value;
			//
			//	if (bcoord.y - h1 < 8) {
			//		const FVec2 blockCoordF = bcoord;
			//		value += 0.9_f * simplex1.value(blockCoordF * BScale);
			//		value += 0.25_f * simplex2.value(blockCoordF * BScale * 2_f);
			//		value += 0.125_f * simplex3.value(blockCoordF * BScale * 4_f);
			//	}
			//	
			//	return std::clamp(value, -1.0_f, 1.0_f);
			//}

			//
			//
			//
			//
			// TODO: can't we just do a gradient clamp((y - h)^4) or something, for sharper falloff?
			//
			//
			//
			//

			const auto bcoord = blockCoord; // TODO: rm if unused
			const bool above = bcoord.y > h2;
			const auto surface = [&]{
				if (above) {
					// Going below below -1 reduces the floating islands/ cancels 
					// `2 / dist` instead of `1 / dist` since we are going [-1, 1] instead of [-1, 0] so the distance is doubled.
					return std::max(-3_f,1_f + (h2 - bcoord.y) * (2_f / 16_f));
					//return 1_f + 2_f * std::max(-1_f, (h1 - bcoord.y) * (1.0f / 8_f));
					//return std::max(-1_f, (h1 - bcoord.y) * (1.0f / 8_f));
				} else {
					return std::max(0_f, 1_f - (h2 - bcoord.y) * (1_f / 128_f));
				}
			}();
			
			const FVec2 blockCoordF = bcoord;
			Float value = 0;
			
			// Cave contribution
			value += 0.9_f * simplex1.value(blockCoordF * BScale);
			value += 0.25_f * simplex2.value(blockCoordF * BScale * 2_f);
			value += 0.125_f * simplex3.value(blockCoordF * BScale * 4_f);
			value = BTrans(value);
			
			// Surface contribution
			value += surface;
			
			// Cave offset/thickness
			value += -BOff;

			value = std::clamp(value, -1.0_f, 1.0_f);
			ENGINE_DEBUG_ASSERT(-1.0_f <= value && value <= 1.0_f);
			return value;
		}
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

		Float getHeight(TERRAIN_GET_HEIGHT_ARGS) {
			// TODO: To avoid the odd bulges in neighboring biomes we should do something like:
			//       `if (rawInfo.id != this.id) { return h0; }`
			const auto half = rawInfo.size / 2;
			const auto off = half - std::abs(rawInfo.biomeRem.x - half);
			const auto hMargin = 30;
			return h0 + off - hMargin;
		}

		Float getBasis(TERRAIN_GET_BASIS_ARGS) {
			const auto xWarp =
				+ 5.0_f * simplex1.value(FVec2{blockCoord} * 0.05f)
				+ 3.0_f * simplex2.value(FVec2{blockCoord} * 0.1f)
				+ 1.5_f * simplex3.value(FVec2{blockCoord} * 0.2f);
			const auto yWarp =
				+ 5.0_f * simplex3.value(FVec2{blockCoord} * 0.05f)
				+ 3.0_f * simplex1.value(FVec2{blockCoord} * 0.1f)
				+ 1.5_f * simplex2.value(FVec2{blockCoord} * 0.2f);
			const auto bcoord = FVec2{blockCoord} + FVec2{xWarp, yWarp};

			const bool above = bcoord.y > h2;
			const auto surface = [&]{
				if (above) {
					return std::max(-1_f, 1_f + (h2 - bcoord.y) * (2_f / 16_f));
				} else {
					return 1_f;
				}
			}();
			
			Float value = std::clamp(surface, -1.0_f, 1.0_f);
			return value;
		}
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

		Float getBasisStrength(TERRAIN_GET_BASIS_STRENGTH_ARGS) {
			//return 0.2_f * simplex1.value(FVec2{blockCoord} * 0.003_f)
			//	 + 0.2_f * simplex2.value(FVec2{blockCoord} * 0.010_f)
			//	 + 0.1_f * simplex3.value(FVec2{blockCoord} * 0.100_f)
			//	 + 0.5_f;
			return 1.0f;
		}

		Float getHeight(TERRAIN_GET_HEIGHT_ARGS) {
			return h0;
		}

		Float getBasis(TERRAIN_GET_BASIS_ARGS) {
			if (blockCoord.y > h2) {
				return -1;
			}

			return 1;
		}
	};
}
