#pragma once

#include <Game/Terrain/Generator.hpp>


#define STAGE_DEF \
	template<StageId Stage> constexpr static bool hasStage = false;\
	template<StageId Stage>\
	ENGINE_INLINE BlockId stage(TERRAIN_STAGE_ARGS) const {\
		static_assert(Stage != Stage, "The requested stage is not defined for this biome.");\
	}

#define STAGE(N)\
	template<> constexpr static bool hasStage<N> = true;\
	template<> ENGINE_INLINE_REL BlockId stage<N>(TERRAIN_STAGE_ARGS) const

namespace Game::Terrain {
	class SimpleBiome {
		public:
			// (required) One or more stages.
			// STAGE_DEF;
			// Stage(1)
			// Stage(2)
			// ...
			// Stage(N)

			// (required) Basis definitions.
			// Float getBasisStrength(TERRAIN_GET_BASIS_STRENGTH_ARGS);
			// Float getBasis(TERRAIN_GET_BASIS_ARGS);

			// (optional) Landmark generation.
			// void getLandmarks(TERRAIN_GET_LANDMARKS_ARGS);
			// void genLandmarks(TERRAIN_GEN_LANDMARKS_ARGS);

		protected:
			ENGINE_INLINE constexpr static Float inGrad(BlockUnit h, BlockUnit y, Float scale) noexcept {
				// This is fine since h will always be within float range of 0.
				return inGrad(static_cast<Float>(h), y, scale);
			}
			
			// TODO: doc, blends from 1 to 0
			ENGINE_INLINE constexpr static Float inGrad(Float h, BlockUnit y, Float scale) noexcept {
				return std::max(0.0_f, 1.0_f - (h - y) * scale);
			}
			
			// TODO: doc, blends from 0 to -1
			ENGINE_INLINE constexpr static Float outGrad(Float h, BlockUnit y, Float scale) noexcept {
				return  std::max(-1.0_f, (h - y) * scale);
			}
	};
}
