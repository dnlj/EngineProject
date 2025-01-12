#pragma once

#include <Game/Terrain/terrain.hpp>


#define STAGE_DEF \
	template<StageId Stage> constexpr static bool hasStage = false;\
	template<StageId Stage>\
	ENGINE_INLINE BlockId stage(TERRAIN_STAGE_ARGS) {\
		static_assert(Stage != Stage, "The requested stage is not defined for this biome.");\
	}

#define STAGE(N)\
	template<> constexpr static bool hasStage<N> = true;\
	template<> ENGINE_INLINE_REL BlockId stage<N>(TERRAIN_STAGE_ARGS)
