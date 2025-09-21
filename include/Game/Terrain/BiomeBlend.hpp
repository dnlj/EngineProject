#pragma once

// Game
#include <Game/Terrain/BiomeWeight.hpp>
#include <Game/Terrain/RawBiomeInfo.hpp>

namespace Game::Terrain {
	class BiomeBlend {
		public:
			RawBiomeInfo info;
			BiomeWeights weights;
			BiomeWeights rawWeights;
	};
}
