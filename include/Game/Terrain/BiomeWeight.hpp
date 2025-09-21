#pragma once

// Game
#include <Game/Terrain/terrain.hpp>

// Engine
#include <Engine/StaticVector.hpp>


namespace Game::Terrain {
	class BiomeWeight {
		public:
			BiomeId id;
			float32 weight;
	};

	using BiomeWeights = Engine::StaticVector<BiomeWeight, 4>;
}
