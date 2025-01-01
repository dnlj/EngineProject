#include <Game/TerrainGenerator.hpp>


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

		// Call generate for each stage. Each will expand the requestion chunk selection
		// appropriately for the following stages.
		Engine::Meta::ForEachInRange<totalStages>::call([&]<auto I>{
			// +1 because stage zero = uninitialized, zero stages have been run yet.
			constexpr static auto CurrentStage = I+1;

			forEachChunkAtStage<CurrentStage>(terrain, request, [&](auto& region, const auto& chunkCoord, const auto& regionIdx) ENGINE_INLINE {
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
		forEachChunkAtStage<totalStages>(terrain, request, [&](Region& region, const ChunkVec& chunkCoord, const RegionIdx& regionIdx) ENGINE_INLINE_REL {
			for (auto const& biomeId : region.getUniqueBiomesApprox(regionIdx)) {
				const auto before = structures.size();
				const auto func = BIOME_GET_DISPATCH(getLandmarks, biomeId);

				if (func) {
					auto const& chunk = region.chunkAt(regionIdx);
					(*func)(biomes, terrain, chunk, chunkCoord, std::back_inserter(structures));
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
				const auto regionCoord = chunkToRegion(chunkCoord);
				auto& region = terrain.getRegion({request.realmId, regionCoord});
				const auto chunkIndex = chunkToRegionIndex(chunkCoord, regionCoord);
				func(region, chunkCoord, chunkIndex);
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
				for (RegionBiomeUnit y = 0; y < biomesPerChunk; ++y) {
					region.biomes[regionBiomeIdx.x + x][regionBiomeIdx.y + y] = maxBiomeWeight(calcBiome(min + BlockVec{x, y} * biomesPerChunk)).id;
				}
			}
		}

		// For each block in the chunk.
		for (BlockUnit x = 0; x < chunkSize.x; ++x) {
			for (BlockUnit y = 0; y < chunkSize.y; ++y) {
				const auto blockCoord = min + BlockVec{x, y};
				BiomeId biomeId;

				// Generate basis only on the first stage
				if constexpr (CurrentStage == 1) {
					const auto basisInfo = calcBasis(blockCoord);
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
					const auto blockId = func(biomes, terrain, chunkCoord, blockCoord, {x, y}, chunk, biomeId, 0);
					chunk.data[x][y] = blockId;
				}
			}
		}
	}

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	// We need to solve the biome/basis blending for all involved biomes. This is needed
	// so we can determine h0/h1 and transition smoothly between them.
	//
	//
	//
	//
	//
	//
	// - May want to consider that temp data we have talked about a coupel of times. _might_ make biome sampling cheaper? Idk
	// - Get biome
	// - Based on distant to edge get up to 4 biomes
	// - Based on distance to edge we can determine weight between 0.5 and 1
	//	 - 0.5 = at edge, 50% biomeA 50% biomeB
	//   - Will need to normalize by the number of total biomes, either 2 or 4
	//   - Might be easer to do math between 0 and 1 then scale, idk
	// - With that we now have weights for each biome
	// - Call biome basis and multiply by weight for final basis
	// - Will need another funciton to determine block weight, exactly the same as we needed in the old map generator.
	//
	//
	//
	//
	//
	//
	//

	template<class... Biomes>
	BiomeRawInfo Generator<Biomes...>::calcBiomeRaw(BlockVec blockCoord) {
		blockCoord.y += biomeOffsetY;

		// TODO: if we simd-ified this we could do all scale checks in a single pass.

		// TODO: Force/manual unroll? We know that N is always small and that would avoid
		//       a scale lookup. This function gets called _a lot_.

		// Determine the scale and sample cell for the biome.
		// Check each scale. Must be ordered largest to smallest.
		BiomeScale scale{};
		while (true) {
			BiomeRawInfo result{ .meta = &biomeScales[+scale] };

			// Rescale the coord so that all scales sample from the same mapping.
			// Originally: cell = glm::floor(blockCoord * biomeScalesInv[result.scale]);
			const auto cell = Engine::Math::divFloor(blockCoord, result.meta->size);
			result.cell = cell.q;
			result.rem = cell.r;

			// No more scales to check. Don't increment so we use the smallest one.
			if ((scale == BiomeScale::_last) || (biomeFreq(result.cell.x, result.cell.y) < result.meta->freq)) {
				result.id = biomePerm(result.cell.x, result.cell.y) % sizeof...(Biomes);
				return result;
			}

			++scale;
		}
	}
	
	template<class... Biomes>
	BiomeWeights Generator<Biomes...>::calcBiomeBlend(BlockVec blockCoord) {
		const auto info = calcBiomeRaw(blockCoord);
		Engine::StaticVector<BiomeWeight, 4> weights{};

		const auto addWeight = [&](BiomeId id, BlockUnit blocks){
			const auto weight = static_cast<Float>(blocks);
			ENGINE_DEBUG_ASSERT(blocks >= 0 && blocks <= biomeBlendDist);

			for (auto& w : weights) {
				if (w.id == id) {
					w.weight += weight;
					return;
				}
			}

			weights.push_back({id, weight});
		};

		addWeight(info.id, biomeBlendDist2);

		// Example:
		//    size = 8
		//   blend = 2
		//    Left = X < 2
		//   Right = X >= 8 - 2; X >= 6
		//   0 1 2 3 4 5 6 7
		//   L L _ _ _ _ R R

		// Distances
		const auto leftD = info.rem.x;
		const auto rightD = info.meta->size - info.rem.x;
		const auto bottomD = info.rem.y;
		const auto topD = info.meta->size - info.rem.y;

		// Conditions
		const auto left = leftD < biomeBlendDist;
		const auto right = rightD <= biomeBlendDist;
		const auto bottom = bottomD < biomeBlendDist;
		const auto top = topD <= biomeBlendDist;

		if (!(left || right || bottom || top)) {
			return weights;
		}

		// Divide by two so the total is 0.5 + d/2 which gives us a value between 0.5 and
		// 1 instead of 0 and 1. At the edges each biome contributes 0.5, not zero.
		addWeight(info.id, std::min({leftD, rightD, bottomD, topD}) / 2);

		// Weights (invert distances)
		const auto leftW = (biomeBlendDist - leftD) / 2;
		const auto rightW = (biomeBlendDist - rightD) / 2;
		const auto bottomW = (biomeBlendDist - bottomD) / 2;
		const auto topW = (biomeBlendDist - topD) / 2;

		// NOTE: This isn't quite right because you get incorrect blending on internal
		//       corners. For example assume you have two biomes: A and B. Then when you
		//       have an internal corner such as:
		//         A A B B
		//         A A B B
		//         B B B B
		//         B B B B
		//       The center corner where A and B meet has an odd gradient. This can be
		//       seen in the the terrain preview with Layer::BiomeBlendWeights. This is
		//       _mostly_ hidden once the basis strength is applied though so its not urgent
		//       to address.

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
			const auto total = std::reduce(weights.cbegin(), weights.cend(), 0.0f, [](Float accum, const auto& value){ return accum + value.weight; });
			ENGINE_DEBUG_ASSERT(total >= biomeBlendDist2 && total <= 2.25*biomeBlendDist, "Incorrect biome weight total: ", total);
		})

		return weights;
	}

	template<class... Biomes>
	BiomeWeights Generator<Biomes...>::calcBiome(BlockVec blockCoord) {
		auto weights = calcBiomeBlend(blockCoord);
		normalizeBiomeWeights(weights);
		ENGINE_DEBUG_ONLY({
			const auto normTotal = std::reduce(weights.cbegin(), weights.cend(), 0.0f, [](Float accum, const auto& value){ return accum + value.weight; });
			ENGINE_DEBUG_ASSERT(std::abs(1.0f - normTotal) <= FLT_EPSILON, "Incorrect normalized biome weight total: ", normTotal);
		});

		// TODO: Does each biome need to specify its own basis strength? Again, this is
		//       the _strength_ of the basis, not the basis itself. I don't think they do.
		//       We could probably just create sizeof(Biomes) simplex samplers and then
		//       use the BiomeId+1/2/3 for the weight of any given biome. Its fine being
		//       defined on the biomes for now though until we have more complete use
		//       cases.

		BIOME_GEN_DISPATCH_REQUIRED(getBasisStrength, Float, TERRAIN_GET_BASIS_ARGS);
		for (auto& biomeWeight : weights) {
			// Output should be between 0 and 1. This is the strength of the basis, not the basis itself.
			const auto getBasisStrength = BIOME_GET_DISPATCH(getBasisStrength, biomeWeight.id);
			const auto basisStr = getBasisStrength(biomes, blockCoord);
			ENGINE_DEBUG_ASSERT(0.0f <= basisStr && basisStr <= 1.0f, "Invalid basis strength value given for biome ", biomeWeight.id, ". Out of range [0, 1].");

			// Multiply with the existing weight to get a smooth transition.
			ENGINE_DEBUG_ASSERT(0.0f <= biomeWeight.weight && biomeWeight.weight <= 1.0f, "Invalid biome blend weight for biome ", biomeWeight.id, ". Out of range [0, 1].");
			biomeWeight.weight *= basisStr;
		}

		return weights;
	}

	template<class... Biomes>
	BasisInfo Generator<Biomes...>::calcBasis(BlockVec blockCoord) {
		const auto weights = calcBiome(blockCoord);
		Float totalBasis = 0.0f;

		BIOME_GEN_DISPATCH_REQUIRED(getBasis, Float, TERRAIN_GET_BASIS_ARGS);
		for (auto& biomeWeight : weights) {
			const auto getBasis = BIOME_GET_DISPATCH(getBasis, biomeWeight.id);
			const auto basis = getBasis(biomes, blockCoord);
			ENGINE_DEBUG_ASSERT(-1.0f <= basis && basis <= 1.0f, "Invalid basis value given for biome ", biomeWeight.id, ". Out of range [-1, 1].");
			totalBasis += biomeWeight.weight * basis;
		}

		return {
			.id = maxBiomeWeight(weights).id,
			.basis = totalBasis,
		};
	}
}
