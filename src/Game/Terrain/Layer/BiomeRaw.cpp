// Game
#include <Game/Terrain/BiomeScale.hpp>
#include <Game/Terrain/Layer/BiomeRaw.hpp>
#include <Game/Terrain/TestGenerator.hpp>


namespace Game::Terrain::Layer {
	void BiomeRaw::request(const Range area, TestGenerator& generator) {
		// No dependencies.
	}

	void BiomeRaw::generate(const Partition chunkCoord, TestGenerator& generator) {
		// TODO: add some kind of empty debug verifier to ensure all get calls are in range of area.

		// No generation/cache.
		// 
		// It is roughly ~15% faster in debug and ~5% in release to just do it inline.
		// This is likely due to the two layers of lookup and area conversion needed
		// between BiomeWeights and BiomeRaw to restructure it like that.
		//
		// If we ever have multiple users of this it might be worth reconsidering, but at
		// the time of writing this the only user of BiomeRaw is BiomeWeight. With only
		// one user it is better to do it inline.
	}

	BiomeRawInfo2 BiomeRaw::get(const Index blockCoord) const noexcept {
		// TODO: if we simd-ified this we could do all scale checks in a single pass.

		// TODO: Could apply some perturb if we want non square biomes. I don't think
		//       Voronoi is worth looking at, its makes it harder to position biomes at surface
		//       level and still produces mostly straight edges, not much better than a square.
		
		// Note that we don't consider biomeScaleOffset or world height (h0) here. In theory we
		// should, in practice its easier and faster to do that in BiomeWeight. Since
		// BiomeWeight is the only user of this class that is fine.

		// Having this manually unrolled instead of a loop is ~15% faster in debug mode
		// and allows the divs to be removed/optimized in release mode. In the loop
		// version, even with full visibility constexpr data, the divs do not get removed
		// even though they theoretically should be able to.

		// Always return the small cell size so that we don't get inflection points when
		// blending biomes. See the blending issue notes in calcBiomeBlend.
		const auto smallCell = Engine::Math::divFloor(blockCoord, biomeScaleSmall.size);
		BiomeRawInfo2 result = {
			.smallCell = smallCell.q,
			.smallRem = smallCell.r,
		};

		{ // Large
			const auto cell = Engine::Math::divFloor(blockCoord, biomeScaleLarge.size);
			if (biomeFreq(cell.q.x, cell.q.y) < biomeScaleLarge.freq) {
				result.id = biomePerm(cell.q.x, cell.q.y) % biomeCount;
				result.size = biomeScaleLarge.size;
				result.biomeCell = cell.q;
				result.biomeRem = cell.r;
				return result;
			}
		}

		{ // Med
			const auto cell = Engine::Math::divFloor(blockCoord, biomeScaleMed.size);
			if (biomeFreq(cell.q.x, cell.q.y) < biomeScaleMed.freq) {
				result.id = biomePerm(cell.q.x, cell.q.y) % biomeCount;
				result.size = biomeScaleMed.size;
				result.biomeCell = cell.q;
				result.biomeRem = cell.r;
				return result;
			}
		}

		{ // Small
			// The small size is used when no other biomes match so frequency doesn't matter.
			static_assert(biomeScaleSmall.freq == 0, "No frequency should be given for the small biome size.");
			result.id = biomePerm(smallCell.q.x, smallCell.q.y) % biomeCount;
			result.size = biomeScaleSmall.size;
			result.biomeCell = smallCell.q;
			result.biomeRem = smallCell.r;
			return result;
		}
	}
}
