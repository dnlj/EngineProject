#pragma once

// Game
#include <Game/Terrain/terrain.hpp>


namespace Game::Terrain {
	class StructureInfo {
		public:
			/** Structure min bounds. Inclusive. */
			BlockVec min;

			/** Structure max bounds. Exclusive. */
			BlockVec max;

			/** A id to identify this structure. Defined by each biome. */
			uint32 id;

			// We would like this to be private so it cant accidentally be modified during
			// biome structure generation, but that causes constructor issues since we use
			// a back_inserter which doesn't play nice with conversion. More hastle than
			// is worth right now.
			/** The biome this structure is in. */
			BiomeId biomeId = {};
	};
}
