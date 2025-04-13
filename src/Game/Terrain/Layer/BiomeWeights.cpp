// Game
#include <Game/Terrain/Layer/BiomeRaw.hpp>
#include <Game/Terrain/Layer/BiomeWeights.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>
#include <Game/Terrain/biomes/all.hpp>


namespace Game::Terrain::Layer {
	void BiomeWeights::request(const Range area, TestGenerator& generator) {
		// TODO: shouldn't this need to consider blendDist as well? Why is this working?
		ENGINE_LOG2("BiomeWeights::request area=({}, {})", area.min, area.max);
		generator.request<WorldBaseHeight>({area.min.x, area.max.x});

		// Note that since BiomeRaw is not cached this call effectively does nothing.
		generator.request<BiomeRaw>(area);
	}

	void BiomeWeights::generate(const Range area, TestGenerator& generator) {
		ENGINE_LOG2("BiomeWeights::generate area=({}, {})", area.min, area.max);
		cache.forEachChunk(area, [&](ChunkVec chunkCoord, auto& chunkStore) ENGINE_INLINE_REL {
			const auto baseBlockCoord = chunkToBlock(chunkCoord);
			for (BlockVec chunkIndex = {0, 0}; chunkIndex.x < chunkSize.x; ++chunkIndex.x) {
				// Theoretically this offset should go in BiomeRaw. In practice its more
				// efficient and easier to do it here. This avoids the need to use a cache
				// in BiomeRaw which makes it ~15% faster and not use any memory.
				const auto offset = biomeScaleOffset + BlockVec{0, generator.get<WorldBaseHeight>(baseBlockCoord.x + chunkIndex.x)};
				for (chunkIndex.y = 0; chunkIndex.y < chunkSize.y; ++chunkIndex.y) {
					const auto blockCoord = baseBlockCoord + chunkIndex - offset;
					chunkStore.at(chunkIndex) = populate(blockCoord, generator);
				}
			}
		});
	}

	const ChunkStore<BiomeBlend>& BiomeWeights::get(const Index chunkCoord) const noexcept {
		const auto regionCoord = chunkToRegion(chunkCoord);
		return cache.at(regionCoord).at(chunkToRegionIndex(chunkCoord, regionCoord));
	}

	[[nodiscard]] BiomeBlend BiomeWeights::populate(BlockVec blockCoord, const TestGenerator& generator) const noexcept {
		BiomeBlend blend = {
			.info = generator.get<BiomeRaw>(blockCoord),
			.weights = {},
		};

		const auto addWeight = [&](BiomeId id, BlockUnit blocks) {
			const auto weight = static_cast<Float>(blocks);
			ENGINE_DEBUG_ASSERT(blocks >= 0 && blocks <= biomeBlendDist);

			for (auto& w : blend.weights) {
				if (w.id == id) {
					//w.weight = 0.8f * weight + 0.2f * w.weight;
					w.weight += weight;
					//w.weight = std::max(weight, w.weight);
					//w.weight = (weight + w.weight) / 2;
					return;
				}
			}

			blend.weights.push_back({id, weight});
		};

		addWeight(blend.info.id, biomeBlendDist2);

		// Example:
		//    size = 8
		//   blend = 2
		//    Left = X < 2
		//   Right = X >= 8 - 2; X >= 6
		//   0 1 2 3 4 5 6 7
		//   L L _ _ _ _ R R

		// Distances
		const auto leftD = blend.info.smallRem.x;
		const auto rightD = biomeScaleSmall.size - blend.info.smallRem.x;
		const auto bottomD = blend.info.smallRem.y;
		const auto topD = biomeScaleSmall.size - blend.info.smallRem.y;

		// Conditions
		const auto left = leftD < biomeBlendDist;
		const auto right = rightD <= biomeBlendDist;
		const auto bottom = bottomD < biomeBlendDist;
		const auto top = topD <= biomeBlendDist;

		if (!(left || right || bottom || top)) {
			return blend;
		}

		// Divide by two so the total is 0.5 + d/2 which gives us a value between 0.5 and
		// 1 instead of 0 and 1. At the edges each biome contributes 0.5, not zero.
		addWeight(blend.info.id, std::min({leftD, rightD, bottomD, topD}) / 2);

		// Weights (invert distances)
		const auto leftW = (biomeBlendDist - leftD) / 2;
		const auto rightW = (biomeBlendDist - rightD) / 2;
		const auto bottomW = (biomeBlendDist - bottomD) / 2;
		const auto topW = (biomeBlendDist - topD) / 2;

		// Blending Issue:
		//   The issue is that when you are on a edge (not at corner) we only check the
		//   biome opposite the current coord, but not the diagonals. This causes the
		//   blending issue when you have a large biome next to a smaller biome on an edge.
		//   
		//   Consider you have three biome areas. A large area (X), a small area (Y) with
		//   the same id as X, and a small area (Z) with a different id than X. The blending
		//   issue occurs on the inner edge where the all three X, Y, and Z meet.
		//   
		//     X X X X X X
		//     X X X X X X
		//     X X O X X X <-- Block O  part of the large biome X. Used a different letter for ease of referencing it.
		//     Z Z Y Y ? ? <-- "?" Is just a placeholder for any biome. We don't care about the biome here.
		//     Z Z Y Y ? ?
		//   
		//   For the blending of block O it will only consider the Y biome below it since it
		//   is not at a corner. This leads to the sharp biome weight transition/step.
		//   
		//   One solution for this is to simply sample the full 3x3 (9 samples total)
		//   centered on the current coord. This works, but slows down generation
		//   _significantly_. This isn't trivial to address using the current logic because
		//   we need the weight of the diagonals, which we don't have here. It is expensive
		//   to get those weights since they aren't based on the size of the current biome
		//   (at block O = X), they are based on Z. The diagonals are only cheap to get at the corners.
		//   
		//   This current logic does work if all biomes are the same scale. One option might
		//   be to make all biomes the same scale and instead have some type of logic to
		//   generate clusters of the same biome.
		//
		//   It is also possible to get the correct weight based on the calcBiomeRaw like
		//   we do above for the current biome, but again, that is q good bit more
		//   calculations and slows things down a lot. We should be able to better than
		//   the naive 3x3 mentioned above though, but again more logic and still more
		//   computations than it is currently.

		if constexpr (true) {
			if (left) { // Left
				addWeight(generator.get<BiomeRaw>({blockCoord.x - biomeBlendDist, blockCoord.y}).id, leftW);
					
				if (bottom) { // Bottom Left
					addWeight(generator.get<BiomeRaw>({blockCoord.x - biomeBlendDist, blockCoord.y - biomeBlendDist}).id, std::min(leftW, bottomW));
				} else if (top) { // Top Left
					addWeight(generator.get<BiomeRaw>({blockCoord.x - biomeBlendDist, blockCoord.y + biomeBlendDist}).id, std::min(leftW, topW));
				}
			} else if (right) { // Right
				addWeight(generator.get<BiomeRaw>({blockCoord.x + biomeBlendDist, blockCoord.y}).id, rightW);
					
				if (bottom) { // Bottom Right
					addWeight(generator.get<BiomeRaw>({blockCoord.x + biomeBlendDist, blockCoord.y - biomeBlendDist}).id, std::min(rightW, bottomW));
				} else if (top) { // Top Right
					addWeight(generator.get<BiomeRaw>({blockCoord.x + biomeBlendDist, blockCoord.y + biomeBlendDist}).id, std::min(rightW, topW));
				}
			}
				
			if (bottom) { // Bottom Center
				addWeight(generator.get<BiomeRaw>({blockCoord.x, blockCoord.y - biomeBlendDist}).id, bottomW);
			} else if (top) { // Top Center
				addWeight(generator.get<BiomeRaw>({blockCoord.x, blockCoord.y + biomeBlendDist}).id, topW);
			}
		} else {
			// Naive 3x3 9 samples. This is the most brute force way possible. We should
			// be able to do a good bit better than this if we want to put in that effort.
			// I wonder if the biome clustering mentioned above would be better and
			// potentally lead to more interesting biome geometry.
			const auto test = [](BlockUnit a, BlockUnit b){
				if (a < 0 && b >= 0) { return b; }
				if (b < 0 && a >= 0) { return a; }
				return std::min(a, b);
			};

			const auto weightAt = [&](BlockVec coord) -> std::pair<BiomeId, BlockUnit> {
				// TODO: Should this be using biomeRem or smallRem? This wasn't updated until later. I think this is okay?
				const auto info = generator.get<BiomeRaw>(coord);
				const auto d1 = info.biomeRem.x;
				const auto d2 = info.size - info.biomeRem.x;
				const auto d3 = info.biomeRem.y;
				const auto d4 = info.size - info.biomeRem.y;
				return {info.id, test(d1, test(d2, test(d3, test(d4, biomeBlendDist))))};
			};

			const auto maybe = [&](BlockVec coord) {
				auto w = weightAt(coord);
				addWeight(w.first, w.second);
			};
		
			leftW;rightW;topW;bottomW;
			blend.weights.clear();
			maybe({blockCoord.x, blockCoord.y});
			maybe({blockCoord.x - biomeBlendDist, blockCoord.y});
			maybe({blockCoord.x - biomeBlendDist, blockCoord.y - biomeBlendDist});
			maybe({blockCoord.x - biomeBlendDist, blockCoord.y + biomeBlendDist});
			maybe({blockCoord.x + biomeBlendDist, blockCoord.y});
			maybe({blockCoord.x + biomeBlendDist, blockCoord.y - biomeBlendDist});
			maybe({blockCoord.x + biomeBlendDist, blockCoord.y + biomeBlendDist});
			maybe({blockCoord.x, blockCoord.y - biomeBlendDist});
			maybe({blockCoord.x, blockCoord.y + biomeBlendDist});
		}

		ENGINE_DEBUG_ONLY({
			// The maximum should be at biomeBlendDist2 away from the corner of a biome.
			// Assuming the bottom left corner, that gives us all four biomes contributing
			// with the weights:
			// 
			//        left = 0.5
			//      bottom = 0.5
			// bottom-left = 0.5
			//        main = 0.5 + 0.25  (0.5 min + 0.25 for halfway between min/max)
			//       total = 0.5 + 0.5 + 0.5 + (0.5 + 0.25) = 2.25
			//                
			const auto total = std::reduce(blend.weights.cbegin(), blend.weights.cend(), 0.0f, [](Float accum, const auto& value){ return accum + value.weight; });
			ENGINE_DEBUG_ASSERT(total >= biomeBlendDist2 && total <= 2.25*biomeBlendDist, "Incorrect biome weight total: ", total);
		})

		return blend;
	}
}
