#pragma once

#include <Game/Terrain/Layer/BiomeRaw.hpp>

namespace Game::Terrain::Layer {
	// TODO: if this is unused we should add the ability to just exclude it?
	template<BiomeId BiomeCount>
	void BiomeRaw<BiomeCount>::request(const Input& area) {
		// No dependants.
	}
	
	template<BiomeId BiomeCount>
	void BiomeRaw<BiomeCount>::generate(BlockVec blockCoord) {
		// TODO: implement caching.
	}
	
	template<BiomeId BiomeCount>
	BiomeRawInfo BiomeRaw<BiomeCount>::get(BlockVec blockCoord) const noexcept {
		// TODO: if we simd-ified this we could do all scale checks in a single pass.

		// TODO: Could apply some perturb if we want non square biomes. I don't think
		//       Voronoi is worth looking at, its makes it harder to position biomes at surface
		//       level and still produces mostly straight edges, not much better than a square.

		// Having this manually unrolled instead of a loop is ~15% faster in debug mode
		// and allows the divs to be removed/optimized in release mode. In the loop
		// version, even with full visibility constexpr data, the divs do not get removed
		// even though they theoretically should be able to.

		// Always return the small cell size so that we don't get inflection points when
		// blending biomes. See the blending issue notes in calcBiomeBlend.
		const auto smallCell = Engine::Math::divFloor(blockCoord, biomeScaleSmall.size);
		BiomeRawInfo result = {
			.smallCell = smallCell.q,
			.smallRem = smallCell.r,
		};

		{ // Large
			const auto cell = Engine::Math::divFloor(blockCoord, biomeScaleLarge.size);
			if (biomeFreq(cell.q.x, cell.q.y) < biomeScaleLarge.freq) {
				result.id = biomePerm(cell.q.x, cell.q.y) % BiomeCount;
				result.size = biomeScaleLarge.size;
				result.biomeCell = cell.q;
				result.biomeRem = cell.r;
				return result;
			}
		}

		{ // Med
			const auto cell = Engine::Math::divFloor(blockCoord, biomeScaleMed.size);
			if (biomeFreq(cell.q.x, cell.q.y) < biomeScaleMed.freq) {
				result.id = biomePerm(cell.q.x, cell.q.y) % BiomeCount;
				result.size = biomeScaleMed.size;
				result.biomeCell = cell.q;
				result.biomeRem = cell.r;
				return result;
			}
		}

		{ // Small
			// The small size is used when no other biomes match so frequency doesn't matter.
			static_assert(biomeScaleSmall.freq == 0, "No frequency should be given for the small biome size.");
			result.id = biomePerm(smallCell.q.x, smallCell.q.y) % BiomeCount;
			result.size = biomeScaleSmall.size;
			result.biomeCell = smallCell.q;
			result.biomeRem = smallCell.r;
			return result;
		}
	}
}
