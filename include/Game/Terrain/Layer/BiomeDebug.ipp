#pragma once

// Game
#include <Game/Terrain/Layer/BiomeDebug.hpp>


//namespace Game::Terrain { namespace {
//	Float heightGrad(BlockUnit hb, BlockUnit yb, BlockUnit fadeb) {
//		// Fade from one to -1 over a distance of fade for y values above h. Useful for
//		// debugging blending. Something like:
//		//   Float getBasis(TERRAIN_GET_BASIS_ARGS) { return heightGrad(h0 + 8, blockCoord.y, 16); }
//		// 
//		// Note that you may will still see sharp transitions if the height discrepancy is
//		// larger than the fade distances. For a truly smooth transition we need to scale
//		// the fade distance with the biome strength or always use a fade distances that
//		// is larger than the maximum discrepancy possible. It should also be noted that
//		// large fade distances are undesirable because the large the fade distance the
//		// more floating islands occur.
//		//
//		// Even if we add a dynamic fade distance based on biome weight we still are
//		// limited by at most a discrepancy of biomeBlendDist since that is the maximum
//		// distance of which we blend. As such we should aim to always have the edge of
//		// biomes roughly at h0. The between blending (biome strength) and this fade
//		// distance we can still deal with some differences, but it is limited.
//
//		Float h = static_cast<Float>(hb);
//		Float y = static_cast<Float>(yb);
//		Float fade = static_cast<Float>(fadeb);
//		// `2 / dist` instead of `1 / dist` since we are going [-1, 1] instead of [-1, 0] so the distance is doubled.
//		if (y > h) { return std::max(-1_f, 1_f + (h - y) * (2_f / fade)); }
//		return 1_f;
//	}
//}}

namespace Game::Terrain::Layer {
	template<uint64 Seed, Float HAmp, Float HFeatScale>
	Float BiomeDebugBaseHeight<Seed, HAmp, HFeatScale>::get(BIOME_HEIGHT_ARGS) const noexcept {
		auto const& simplex1 = generator.shared<BiomeDebugSharedData<Seed>>().simplex1;
		return h0 + HAmp * simplex1.value(blockCoordX * HFeatScale, 0); // TODO: 1d simplex
	}

	template<uint64 Seed>
	Float BiomeDebugBasisStrength<Seed>::get(BIOME_BASIS_STRENGTH_ARGS) const noexcept {
		auto const& shared = generator.shared<BiomeDebugSharedData<Seed>>();
		auto const& simplex1 = shared.simplex1;
		auto const& simplex2 = shared.simplex2;
		auto const& simplex3 = shared.simplex3;

		// These need to be tuned based on biome scales blend dist or else you can get odd clipping type issues.
		return 0.2_f * simplex1.value(FVec2{blockCoord} * 0.003_f)
				+ 0.2_f * simplex2.value(FVec2{blockCoord} * 0.010_f)
				+ 0.1_f * simplex3.value(FVec2{blockCoord} * 0.100_f)
				+ 0.5_f;
	}

	template<uint64 Seed, Float HAmp, Float HFeatScale, Float BScale, Float BOff, auto BTrans>
	Float BiomeDebugBasis<Seed, HAmp, HFeatScale, BScale, BOff, BTrans>::get(BIOME_BASIS_ARGS) const noexcept {
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
			
			// TODO: can't we just do a gradient clamp((y - h)^4) or something, for sharper falloff?
			
			auto const& shared = generator.shared<BiomeDebugSharedData<Seed>>();
			auto const& simplex1 = shared.simplex1;
			auto const& simplex2 = shared.simplex2;
			auto const& simplex3 = shared.simplex3;

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
}
