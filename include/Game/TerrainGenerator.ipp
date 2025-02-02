#include <Game/TerrainGenerator.hpp>


// TODO: Notes on terms to document:
//       Basis - The shape/topology of the terrain regardless of the specific blocks. Needed to combine terrain when blending between biomes.
//       Basis Strength - Needed blending biomes. How "strong" is the biome at this point. Allows us to interpolate and smooth between biomes.


#define BIOME_GET_DISPATCH_NAME(Name) _engine_BiomeGenFuncTable_##Name
#define BIOME_GET_DISPATCH(Name, BiomeId) BIOME_GET_DISPATCH_NAME(Name)[BiomeId]

/**
 * Generate a function lookup table for biomes.
 * Some biomes may choose to not define the function. In that case BIOME_GET_DISPATCH will
 * return nullptr.
 */
#define BIOME_GEN_DISPATCH(Name, ReturnType, ...) \
	using _engine_BiomeGenFunc_##Name = ReturnType (*)(std::tuple<Biomes...>& biomes, __VA_ARGS__); \
	constexpr static auto _engine_BiomeGenFuncCall_##Name = []<class Biome>() constexpr -> _engine_BiomeGenFunc_##Name { \
		if constexpr (requires { &Biome::Name; }) { \
			return []<class... Args>(std::tuple<Biomes...>& biomes, Args... args) ENGINE_INLINE { \
				static_assert(std::same_as<ReturnType, decltype(std::get<Biome>(biomes).Name(args...))>, "Biome dispatch function has incorrect return type."); \
				ENGINE_INLINE_CALLS { return std::get<Biome>(biomes).Name(args...); }\
			}; \
		} else { \
			return nullptr; \
		} \
	}; \
	constexpr static _engine_BiomeGenFunc_##Name BIOME_GET_DISPATCH(Name,) = { _engine_BiomeGenFuncCall_##Name.template operator()<Biomes>()... };

/**
 * Generate a function lookup table for biomes with the requirement that all biomes define the function.
 * @see BIOME_GEN_DISPATCH
 */
#define BIOME_GEN_DISPATCH_REQUIRED(Name, ReturnType, ...) \
	BIOME_GEN_DISPATCH(Name, ReturnType, __VA_ARGS__); \
	static_assert(std::ranges::all_of(BIOME_GET_DISPATCH_NAME(Name), [](auto* ptr){ return ptr != nullptr; }), "One or more biomes are missing the required " #Name " function.");

/**
 * Generate a function lookup table for biomes for a template function.
 * @param CallName the symbol for the template function with populated template arguments.
 * @see BIOME_GEN_DISPATCH
 */
#define BIOME_GEN_DISPATCH_T(Name, CallName, ReturnType, ...) \
	using _engine_BiomeGenFunc_##Name = ReturnType (*)(std::tuple<Biomes...>& biomes, __VA_ARGS__); \
	constexpr static auto _engine_BiomeGenFuncCall_##Name = []<class Biome>() constexpr -> _engine_BiomeGenFunc_##Name { \
		if constexpr (requires { &Biome::template CallName; }) { \
			return []<class... Args>(std::tuple<Biomes...>& biomes, Args... args) ENGINE_INLINE { \
				ENGINE_INLINE_CALLS { return std::get<Biome>(biomes).template CallName(args...); }\
			}; \
		} else { \
			return nullptr; \
		} \
	}; \
	constexpr static _engine_BiomeGenFunc_##Name BIOME_GET_DISPATCH(Name,) = { _engine_BiomeGenFuncCall_##Name.template operator()<Biomes>()... };


namespace Game::Terrain {
	template<class... Biomes>
	void Generator<Biomes...>::generate1(Terrain& terrain, const Request& request) {
		// TODO: add biome stages
		// TODO: add h0/h1 stages
		// - Generate stages.
		//   - Stage 1, Stage 2, ..., Stage N.
		// - Generate candidate features for all biomes in the request.
		// - Cull candidate features based on point (or AABB?) and realized biome.
		// - Generate non-culled features based on resolved biome.
		// - Generate decorations.
		//   - Basically same as stages.
		//   - Provides a place to smooth/integrate features and terrain.
		//   - Moss, grass tufts, cobwebs, chests/loot, etc.
		//   - Do these things really need extra passes? Could this be done during the initial stages and feature generation?

		// TODO: Should this be cached (or stored?) on the terrain? Maybe cache on the
		//       generator? Right now we re-generate for every request.

		// TODO: does this need ot be +1 chunk? Can't we just do +1 block?
		setupHeightCaches(chunkToBlock(request.minChunkCoord).x, chunkToBlock(request.maxChunkCoord + ChunkVec{1, 0}).x);

		// Call generate for each stage. Each will expand the requestion chunk selection
		// appropriately for the following stages.
		Engine::Meta::ForEachInRange<totalStages>::call([&]<auto I>{
			// +1 because stage zero = uninitialized, zero stages have been run yet.
			constexpr static auto CurrentStage = I+1;

			forEachChunkAtStage<CurrentStage>(terrain, request, [&](Region& region, const UniversalRegionCoord& regionCoord, const RegionIdx& regionIdx, const ChunkVec& chunkCoord) ENGINE_INLINE {
				auto& stage = region.stages[regionIdx.x][regionIdx.y];
				
				if (stage < CurrentStage) {
					ENGINE_DEBUG_ASSERT(stage == CurrentStage - 1);
					generateChunk<CurrentStage>(terrain, region, regionIdx, chunkCoord, region.chunkAt(regionIdx));
					//ENGINE_LOG2("Generate Chunk: {}", chunkCoord);
					++stage;
				}
			});
		});

		BIOME_GEN_DISPATCH(getLandmarks, void, TERRAIN_GET_LANDMARKS_ARGS);
		BIOME_GEN_DISPATCH(genLandmarks, void, TERRAIN_GEN_LANDMARKS_ARGS);

		std::vector<StructureInfo> structures;
		forEachChunkAtStage<totalStages>(terrain, request, [&](Region& region, const UniversalRegionCoord& regionCoord, const RegionIdx& regionIdx, const ChunkVec& chunkCoord) ENGINE_INLINE_REL {
			for (auto const& biomeId : region.getUniqueBiomesApprox(regionIdx)) {
				const auto before = structures.size();
				const auto func = BIOME_GET_DISPATCH(getLandmarks, biomeId);

				if (func) {
					auto const& chunk = region.chunkAt(regionIdx);
					(*func)(biomes, terrain, regionCoord, regionIdx, chunkCoord, chunk, h0Cache, std::back_inserter(structures));
				}

				// TODO: could this be done with a custom back_inserter instead of an extra loop after the fact?
				const auto after = structures.size();
				if (after != before) {
					for (auto i = before; i < after; ++i) {
						auto& info = structures[i];
						ENGINE_DEBUG_ASSERT(info.min.x <= info.max.x);
						ENGINE_DEBUG_ASSERT(info.min.y <= info.max.y);
						info.biomeId = biomeId;
						info.realmId = request.realmId;
					}
				}
			}
		});

		// TODO: Consider using a BSP tree, quad tree, BVH, etc. some spacial structure.
		
		// TODO: How do we want to resolve conflicts? If we just go first come first serve
		//       then a spacial structure isn't needed, you could just check when
		//       inserting. If its not FCFS then we need to keep all boxes and resovle at
		//       the end or else you might over-cull. For exampel say you have a large
		//       structure that later gets removed by a higher priority small structure.
		//       Since the large structure is now removed there could be other structures
		//       which are now valid since the larger structure no longer exists and
		//       doens't collide.

		//
		//
		// TODO: Cull overlaps
		//
		//
		//if (!structures.empty()) {
		//	auto cur = structures.begin();
		//	const auto end = structures.end() - 1;
		//
		//	while (cur != end) {
		//		const auto next = cur + 1;
		//		while
		//	}
		//}

		// TODO: Theoretically a structure could overlap into unloaded/unfinished chunks.
		//       Don't we need to check and then generate those chunks here? Post culling.
		//       Or add some extra buffer to the initial request based on the largest known structure. <-- Easiest, but worse perf.
		//       
		//       Maybe at this point we could issue additional generation requests to generate
		//       those needed chunks. This is probably be the best option, but we need to add a
		//       way to generate those structs up to hte block stage and then not to structs.
		//       Right now we already store the stage so we just need to add a stage for structs
		//       and that should be fairly doable.

		for (const auto& structInfo : structures) {
			if (structInfo.generate) {
				const auto func = BIOME_GET_DISPATCH(genLandmarks, structInfo.biomeId);
				ENGINE_DEBUG_ASSERT(func, "Attempting to generate landmark in biome that does not support them.");
				func(biomes, terrain, structInfo);
			}
		}
	}

	template<class... Biomes>
	template<StageId CurrentStage, class Func>
	void Generator<Biomes...>::forEachChunkAtStage(Terrain& terrain, const Request& request, Func&& func) {
		constexpr ChunkUnit offset = totalStages - CurrentStage;
		const auto min = request.minChunkCoord - offset;
		const auto max = request.maxChunkCoord + offset;

		// TODO: if we split request on region bounds we can avoid the repeated region lookup.
		for (auto chunkCoord = min; chunkCoord.x <= max.x; ++chunkCoord.x) {
			for (chunkCoord.y = min.y; chunkCoord.y <= max.y; ++chunkCoord.y) {
				const UniversalRegionCoord regionCoord = {request.realmId, chunkToRegion(chunkCoord)};
				auto& region = terrain.getRegion(regionCoord);
				const auto regionIdx = chunkToRegionIndex(chunkCoord, regionCoord.pos);
				func(region, regionCoord, regionIdx, chunkCoord);
			}
		}
	}

	template<class... Biomes>
	template<StageId CurrentStage>
	void Generator<Biomes...>::generateChunk(Terrain& terrain, Region& region, const RegionIdx regionIdx, const ChunkVec chunkCoord, Chunk& chunk) {
		// TODO: finish comment
		// We need to do the biome stages this way (via func ptr) because of the
		// artificial limitation C++ imposes that no template function, _including fully
		// specialized template functions_, cannot be virtual. Since they can't be
		// specialized and virtual we have no way to pass the current stage other than to
		// resolve it at runtime, which would involve at minimum an additional ptr chase
		// and two indexes from the vtable and stage resolution. We know... TODO: fin
		BIOME_GEN_DISPATCH_T(stage, stage<CurrentStage>, BlockId, TERRAIN_STAGE_ARGS);

		const auto min = chunkToBlock(chunkCoord);
		const auto max = min + chunkSize;

		// Populate biomes in chunk.
		// TODO: We may want a temporary block granularity biome lookup for structure and decoration generation.
		//       I think there is probably other temp info we would want only during generation also, h0, h1, biome, etc.
		static_assert(CurrentStage != 0, "Stage zero means no generation. This is a bug.");
		if constexpr (CurrentStage == 1) {
			const auto regionBiomeIdx = regionIdxToRegionBiomeIdx(regionIdx);
			for (RegionBiomeUnit x = 0; x < biomesPerChunk; ++x) {
				const auto h0 = h0Cache.get(min.x + x);
				for (RegionBiomeUnit y = 0; y < biomesPerChunk; ++y) {
					region.biomes[regionBiomeIdx.x + x][regionBiomeIdx.y + y] = maxBiomeWeight(calcBiome(min + BlockVec{x, y} * biomesPerChunk, h0).weights).id;
				}
			}
		}

		// TODO: Find a better solution. This is currently a hack as it isn't populated by
		//       any stage other than the first. We either need to split the first stage
		//       to have its own args or cache the biome info per generate call? Since we
		//       don't need it currently in any of the following stages I'm leaning toward
		//       the first?
		BasisInfo basisInfo{};

		// For each block in the chunk.
		for (BlockUnit x = 0; x < chunkSize.x; ++x) {
			const auto h0 = h0Cache.get(min.x + x);
			for (BlockUnit y = 0; y < chunkSize.y; ++y) {
				const auto blockCoord = min + BlockVec{x, y};
				BiomeId biomeId;

				// Generate basis only on the first stage
				if constexpr (CurrentStage == 1) {
					basisInfo = calcBasis(blockCoord, h0);

					if (basisInfo.basis <= 0.0_f) {
						// TODO: is there a reason we don't just default to air as 0/{}? Do we need BlockId::None?
						chunk.data[x][y] = BlockId::Air;
						continue;
					}
					biomeId = basisInfo.id;
				} else {
					biomeId = maxBiomeWeight(calcBiome(blockCoord)).id;
				}

				const auto func = BIOME_GET_DISPATCH(stage, biomeId);
				if (func) {
					const auto blockId = func(biomes, terrain, chunkCoord, blockCoord, {x, y}, chunk, biomeId, h0, basisInfo);
					chunk.data[x][y] = blockId;
				}
			}
		}
	}

	template<class... Biomes>
	void Generator<Biomes...>::setupHeightCaches(const BlockUnit minBlock, const BlockUnit maxBlock) {
		// We need to include the biome blend distance for h0 for biome sampling in calcBiomeRaw.
		h0Cache.reset(minBlock - biomeBlendDist, maxBlock + biomeBlendDist);
		h2Cache.reset(minBlock, maxBlock);

		// H2 is currently populated as needed in calcBasis. Would be nice to do it here
		// but it require full biome info which is expensive. Would need to bench and see
		// if it is worth it.

		{
			const auto h0Min = h0Cache.getMinBlock();
			const auto h0Max = h0Cache.getMaxBlock();
			for (BlockUnit blockCoord = h0Min; blockCoord < h0Max; ++blockCoord) {
				// TODO: use _f for Float. Move from TerrainPreview.
				// TODO: keep in mind that this is +- amplitude, and for each octave we increase the contrib;

				//
				//
				// TODO: tune + octaves, atm this is way to steep
				//
				//
				h0Cache.get(blockCoord) = static_cast<BlockUnit>(500 * simplex1.value(blockCoord * 0.00005f, 0));
			}
		}

		
		{
			const auto h2Min = h2Cache.getMinBlock();
			const auto h2Max = h2Cache.getMaxBlock();
			for (BlockUnit blockCoord = h2Min; blockCoord < h2Max; ++blockCoord) {
				const auto h0 = h0Cache.get(blockCoord);
				const Float h0F = static_cast<Float>(h0);
				const auto& blend = calcBiome({blockCoord, h0}, h0);
				Float h2 = 0;

				// Its _very_ important that we use the _raw weights_ for blending height
				// transitions. If we don't then you will get alterations between different biome
				// heights and huge floating islands due the basisStrength.
				for (auto& biomeWeight : blend.rawWeights) {
					BIOME_GEN_DISPATCH_REQUIRED(getHeight, Float, TERRAIN_GET_HEIGHT_ARGS);
					const auto getHeight = BIOME_GET_DISPATCH(getHeight, biomeWeight.id);
					const auto h1 = getHeight(biomes, {blockCoord, h0}, h0F, blend.info, biomeWeight.weight);
					h2 += biomeWeight.weight * h1;
				}

				h2Cache.get(blockCoord) = static_cast<BlockUnit>(std::floor(h2));
			}
		}
	}

	template<class... Biomes>
	BiomeRawInfo Generator<Biomes...>::calcBiomeRaw(BlockVec blockCoord) {
		// TODO: if we simd-ified this we could do all scale checks in a single pass.

		// TODO: Could apply some perturb if we want non square biomes. I don't think
		//       Voronoi is worth looking at, its makes it harder to position biomes at surface
		//       level and still produces mostly straight edges, not much better than a square.

		// Having this manually unrolled instead of a loop is ~15% faster in debug mode
		// and allows the divs to be removed/optimized in release mode. In the loop
		// version, even with full visibility constexpr data, the divs do not get removed
		// even though the theoretically should be able to.

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
				result.id = biomePerm(cell.q.x, cell.q.y) % sizeof...(Biomes);
				result.size = biomeScaleLarge.size;
				result.biomeCell = cell.q;
				result.biomeRem = cell.r;
				return result;
			}
		}

		{ // Med
			const auto cell = Engine::Math::divFloor(blockCoord, biomeScaleMed.size);
			if (biomeFreq(cell.q.x, cell.q.y) < biomeScaleMed.freq) {
				result.id = biomePerm(cell.q.x, cell.q.y) % sizeof...(Biomes);
				result.size = biomeScaleMed.size;
				result.biomeCell = cell.q;
				result.biomeRem = cell.r;
				return result;
			}
		}

		{ // Small
			// The small size is used when no other biomes match so frequency doesn't matter.
			static_assert(biomeScaleSmall.freq == 0, "No frequency should be given for the small biome size.");
			result.id = biomePerm(smallCell.q.x, smallCell.q.y) % sizeof...(Biomes);
			result.size = biomeScaleSmall.size;
			result.biomeCell = smallCell.q;
			result.biomeRem = smallCell.r;
			return result;
		}

	}
	
	template<class... Biomes>
	BiomeBlend Generator<Biomes...>::calcBiomeBlend(BlockVec blockCoord, const BlockUnit h0) {
		// Offset the coord by the scale offset so biomes are roughly centered on {0, 0}
		blockCoord -= biomeScaleOffset + h0;

		BiomeBlend blend = {
			.info = calcBiomeRaw(blockCoord),
			.weights = {},
		};

		const auto addWeight = [&](BiomeId id, BlockUnit blocks){
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
				addWeight(calcBiomeRaw({blockCoord.x - biomeBlendDist, blockCoord.y}).id, leftW);
					
				if (bottom) { // Bottom Left
					addWeight(calcBiomeRaw({blockCoord.x - biomeBlendDist, blockCoord.y - biomeBlendDist}).id, std::min(leftW, bottomW));
				} else if (top) { // Top Left
					addWeight(calcBiomeRaw({blockCoord.x - biomeBlendDist, blockCoord.y + biomeBlendDist}).id, std::min(leftW, topW));
				}
			} else if (right) { // Right
				addWeight(calcBiomeRaw({blockCoord.x + biomeBlendDist, blockCoord.y}).id, rightW);
					
				if (bottom) { // Bottom Right
					addWeight(calcBiomeRaw({blockCoord.x + biomeBlendDist, blockCoord.y - biomeBlendDist}).id, std::min(rightW, bottomW));
				} else if (top) { // Top Right
					addWeight(calcBiomeRaw({blockCoord.x + biomeBlendDist, blockCoord.y + biomeBlendDist}).id, std::min(rightW, topW));
				}
			}
				
			if (bottom) { // Bottom Center
				addWeight(calcBiomeRaw({blockCoord.x, blockCoord.y - biomeBlendDist}).id, bottomW);
			} else if (top) { // Top Center
				addWeight(calcBiomeRaw({blockCoord.x, blockCoord.y + biomeBlendDist}).id, topW);
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
				const auto info = calcBiomeRaw(coord);
				const auto d1 = info.rem.x;
				const auto d2 = info.meta->size - info.rem.x;
				const auto d3 = info.rem.y;
				const auto d4 = info.meta->size - info.rem.y;
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

	template<class... Biomes>
	BiomeBlend Generator<Biomes...>::calcBiome(BlockVec blockCoord, const BlockUnit h0) {
		auto blend = calcBiomeBlend(blockCoord, h0);
		normalizeBiomeWeights(blend.weights);
		blend.rawWeights = blend.weights;
		ENGINE_DEBUG_ONLY({
			const auto normTotal = std::reduce(blend.weights.cbegin(), blend.weights.cend(), 0.0f, [](Float accum, const auto& value){ return accum + value.weight; });
			ENGINE_DEBUG_ASSERT(std::abs(1.0f - normTotal) <= FLT_EPSILON, "Incorrect normalized biome weight total: ", normTotal);
		});


		// TODO: Does each biome need to specify its own basis strength? Again, this is
		//       the _strength_ of the basis, not the basis itself. I don't think they do.
		//       We could probably just create sizeof(Biomes) simplex samplers and then
		//       use the BiomeId+1/2/3 for the weight of any given biome. Its fine being
		//       defined on the biomes for now though until we have more complete use
		//       cases.

		BIOME_GEN_DISPATCH_REQUIRED(getBasisStrength, Float, TERRAIN_GET_BASIS_STRENGTH_ARGS);
		for (auto& biomeWeight : blend.weights) {
			// Output should be between 0 and 1. This is the strength of the basis, not the basis itself.
			const auto getBasisStrength = BIOME_GET_DISPATCH(getBasisStrength, biomeWeight.id);
			const auto basisStr = getBasisStrength(biomes, blockCoord);
			ENGINE_DEBUG_ASSERT(0.0f <= basisStr && basisStr <= 1.0f, "Invalid basis strength value given for biome ", biomeWeight.id, ". Out of range [0, 1].");

			// Multiply with the existing weight to get a smooth transition.
			ENGINE_DEBUG_ASSERT(0.0f <= biomeWeight.weight && biomeWeight.weight <= 1.0f, "Invalid biome blend weight for biome ", biomeWeight.id, ". Out of range [0, 1].");
			biomeWeight.weight *= basisStr;
		}

		return blend;
	}

	template<class... Biomes>
	BasisInfo Generator<Biomes...>::calcBasis(const BlockVec blockCoord, const BlockUnit h0) {
		const auto blend = calcBiome(blockCoord, h0);
		const Float h0F = static_cast<Float>(h0);
		const Float h2 = static_cast<Float>(h2Cache.get(blockCoord.x));

		Float totalBasis = 0;
		for (auto& biomeWeight : blend.weights) {
			BIOME_GEN_DISPATCH_REQUIRED(getBasis, Float, TERRAIN_GET_BASIS_ARGS);
			const auto getBasis = BIOME_GET_DISPATCH(getBasis, biomeWeight.id);
			const auto basis = getBasis(biomes, blockCoord, h0F, h2, blend.info, biomeWeight.weight);

			// This is a _somewhat_ artificial limitation. A basis doesn't _need_ to be
			// between [-1, 1], but all biomes should have roughly the same range. If one
			// is [-100, 100] and another is [-1, 1] they won't blend well since the one
			// with the larger range will always dominate regardless of the blend weight.
			// Keeping things normalized avoids that.
			ENGINE_DEBUG_ASSERT(-1.0_f <= basis && basis <= 1.0_f, "Invalid basis value given for biome ", biomeWeight.id, ". Out of range [-1, 1].");
			totalBasis += biomeWeight.weight * basis;
		}

		const auto maxBiome = maxBiomeWeight(blend.weights);
		return {
			.id = maxBiome.id,
			.weight = maxBiome.weight,
			.basis = totalBasis,
			.h2 = h2,
			.rawInfo = blend.info,
			.rawWeight = maxBiomeWeight(blend.rawWeights).weight,
		};
	}
}
