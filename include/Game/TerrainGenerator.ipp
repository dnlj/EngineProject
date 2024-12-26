#include <Game/TerrainGenerator.hpp>


#define BIOME_GET_DISPATCH(Name, BiomeId) _engine_BiomeGenFuncTable_##Name[BiomeId]
#define BIOME_GEN_DISPATCH(Name, ...) \
	using _engine_BiomeGenFunc_##Name = void (*)(std::tuple<Biomes...>& biomes, __VA_ARGS__); \
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
					ENGINE_LOG2("Generate Chunk: {}", chunkCoord);
					++stage;
				}
			});
		});

		BIOME_GEN_DISPATCH(getLandmarks, TERRAIN_GET_LANDMARKS_ARGS);
		BIOME_GEN_DISPATCH(genLandmarks, TERRAIN_GEN_LANDMARKS_ARGS);

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
		static_assert(CurrentStage != 0, "Stage zero means no generation. This is a bug.");
		if constexpr (CurrentStage == 1) {
			const auto regionBiomeIdx = regionIdxToRegionBiomeIdx(regionIdx);
			for (RegionBiomeUnit x = 0; x < biomesPerChunk; ++x) {
				for (RegionBiomeUnit y = 0; y < biomesPerChunk; ++y) {
					region.biomes[regionBiomeIdx.x + x][regionBiomeIdx.y + y] = calcBiome(min + BlockVec{x, y} * biomesPerChunk).id;
				}
			}
		}

		// For each block in the chunk.
		for (BlockUnit x = 0; x < chunkSize.x; ++x) {
			for (BlockUnit y = 0; y < chunkSize.y; ++y) {
				const auto blockCoord = min + BlockVec{x, y};
				const auto biomeInfo = calcBiome(blockCoord);

				const auto func = BIOME_GET_DISPATCH(stage, biomeInfo.id);
				if (func) {
					const auto blockId = func(biomes, terrain, chunkCoord, blockCoord, {x, y}, chunk, biomeInfo, 0);
					chunk.data[x][y] = blockId;
				}
			}
		}
	}

	template<class... Biomes>
	BiomeInfo Generator<Biomes...>::calcBiome(BlockVec blockCoord) {
		blockCoord.y += biomeOffsetY;

		// TODO: if we simd-ified this we could do all scale checks in a single pass.

		// TODO: Force/manual unroll? We know that N is always small and that would avoid
		//       a scale lookup. This function gets called _a lot_.
		BiomeInfo result{};

		// Determine the scale and sample cell for the biome.
		// Check each scale. Must be ordered largest to smallest.
		while (true) {
			const auto& scale = biomeScales[+result.scale];

			// Rescale the coord so that all scales sample from the same mapping.
			// Originally: cell = glm::floor(blockCoord * biomeScalesInv[result.scale]);
			result.cell = Engine::Math::divFloor(blockCoord, scale.scale).q;

			if (biomeFreq(result.cell.x, result.cell.y) < scale.freq) { break; }

			// No more scales to check. Don't increment so we use the smallest one.
			if (result.scale == BiomeScale::_last) { break; }
			++result.scale;
		}

		result.id = biomePerm(result.cell.x, result.cell.y) % sizeof...(Biomes);
		return result;
	}
}
