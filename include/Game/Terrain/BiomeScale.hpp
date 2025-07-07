#pragma once

namespace Game::Terrain {
	class BiomeScaleMeta {
		public:
			BlockUnit size;

			/** Percentage frequency out of [0, 255]. */
			uint8 freq;
	};
	
	// Each scale must be divisible by the previous depth. If they aren't then you will
	// get small "remainder" biomes around the edges of large ones which isn't a technical
	// issue, but it looks very strange to have a very small biome.
	//
	// We want to use divisions by three so that each biome can potentially spawn at surface
	// level. If we use two then only the first depth will be at surface level and all others
	// will be above or below it. This is due to the division by two for the biomeScaleOffset.
	// We need division by two because we want biomes evenly centered on y=0.
	constexpr inline BiomeScaleMeta biomeScaleSmall = { .size = 600, .freq = 0 };// TODO: use pow 2? 512?
	constexpr inline BiomeScaleMeta biomeScaleMed = { .size = biomeScaleSmall.size * 3, .freq = 20 };
	constexpr inline BiomeScaleMeta biomeScaleLarge = { .size = biomeScaleMed.size * 3, .freq = 20 };

	constexpr inline int32 biomeBlendDist = 200;
	constexpr inline int32 biomeBlendDist2 = biomeBlendDist / 2;
	static_assert(2 * biomeBlendDist2 == biomeBlendDist);
	static_assert(biomeBlendDist <= biomeScaleSmall.size / 2, "Biome blend distances cannot be larger than half the minimum biome size.");

	// The main idea is that we want the biomes to be centered on {0,0} so that surface
	// level (Y) is in the middle of a biome instead of right on the edge. Same for X, we
	// want the center of the world to be in the middle instead of on the edge.
	//
	// This is why its important that each biome scale is a 3x multiple of the previous.
	// That makes it so that when you divide by two all biomes are centered regardless of scale.
	constexpr static BlockVec biomeScaleOffset = {
		biomeScaleLarge.size / 2,
		// TODO: recalc the +200 part, that is from the old map gen
		// Experimentally terrain surface is around 100-300
		biomeScaleLarge.size / 2,// + 200.0
	};
}
