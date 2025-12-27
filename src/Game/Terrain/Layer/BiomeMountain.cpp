#pragma once

// Game
#include <Game/Terrain/Layer/BiomeMountain.hpp>
#include <Game/Terrain/TestGenerator.hpp>

namespace Game::Terrain::Layer {
	Float BiomeMountainHeight::get(BIOME_HEIGHT_ARGS) const noexcept {
		// TODO: To avoid the odd bulges in neighboring biomes we should do something like:
		//       `if (rawInfo.id != this.id) { return h0; }`
		const auto half = rawInfo.size / 2;
		const auto off = half - std::abs(rawInfo.biomeRem.x - half);
		const auto hMargin = 30;
		return h0 + off - hMargin;
	}

	Float BiomeMountainBasis::get(BIOME_BASIS_ARGS) const noexcept {
		const auto& shared = generator.shared<BiomeDebugMountain::SharedData>();
		const auto& simplex1 = shared.simplex1;
		const auto& simplex2 = shared.simplex2;
		const auto& simplex3 = shared.simplex3;

		const auto xWarp =
			+ 5.0_f * simplex1.value(blockCoordF * 0.05f)
			+ 3.0_f * simplex2.value(blockCoordF * 0.1f)
			+ 1.5_f * simplex3.value(blockCoordF * 0.2f);

		const auto yWarp =
			+ 5.0_f * simplex3.value(blockCoordF * 0.05f)
			+ 3.0_f * simplex1.value(blockCoordF * 0.1f)
			+ 1.5_f * simplex2.value(blockCoordF * 0.2f);

		const auto bcoord = blockCoordF + FVec2{xWarp, yWarp};

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
}
