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

			/**
			 * A id to identify this structure. Defined by each biome. This is not necessarily
			 * unique and may be used more as an type or variant.
			 */
			uint32 id;

			// We would like this to be private so it cant accidentally be modified during
			// biome structure generation, but that causes constructor issues since we use
			// a back_inserter which doesn't play nice with conversion. More hastle than
			// is worth right now.
			/** The biome this structure is in. */
			BiomeId biomeId = {};

			ENGINE_INLINE bool operator==(const StructureInfo& right) const noexcept = default;
	};
}

template<>
struct Engine::Hash<Game::Terrain::StructureInfo> {
	[[nodiscard]] ENGINE_INLINE uintz operator()(const Game::Terrain::StructureInfo& val) const {
		uintz result = 0;
		hashCombine(result, Engine::hash(val.min));
		hashCombine(result, Engine::hash(val.max));
		hashCombine(result, Engine::hash(val.id));
		hashCombine(result, Engine::hash(val.biomeId));
		return result;
	}
};
