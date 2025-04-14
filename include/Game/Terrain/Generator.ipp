#include <Game/Terrain/Generator.hpp>


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
	using _engine_BiomeGenFunc_##Name = ReturnType (*)(const std::tuple<Biomes...>& biomes, __VA_ARGS__); \
	constexpr static auto _engine_BiomeGenFuncCall_##Name = []<class Biome>() constexpr -> _engine_BiomeGenFunc_##Name { \
		if constexpr (requires { &Biome::Name; }) { \
			return []<class... Args>(const std::tuple<Biomes...>& biomes, Args... args) ENGINE_INLINE { \
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
	using _engine_BiomeGenFunc_##Name = ReturnType (*)(const std::tuple<Biomes...>& biomes, __VA_ARGS__); \
	constexpr static auto _engine_BiomeGenFuncCall_##Name = []<class Biome>() constexpr -> _engine_BiomeGenFunc_##Name { \
		if constexpr (requires { &Biome::template CallName; }) { \
			return []<class... Args>(const std::tuple<Biomes...>& biomes, Args... args) ENGINE_INLINE { \
				ENGINE_INLINE_CALLS { return std::get<Biome>(biomes).template CallName(args...); }\
			}; \
		} else { \
			return nullptr; \
		} \
	}; \
	constexpr static _engine_BiomeGenFunc_##Name BIOME_GET_DISPATCH(Name,) = { _engine_BiomeGenFuncCall_##Name.template operator()<Biomes>()... };


namespace Game::Terrain {
	template<class... Biomes>
	void Generator<Biomes...>::generate(Terrain& terrain, const Request& request) {
		// TODO: add biome stages
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

		// TODO: I think the offset accounts for biomeBlendDistance, that should no longer
		//       be needed here once everything is converted to layers and setupHeightCaches
		//       is removed.
		// 
		// TODO: avoid name conflict with arguments `request`
		//
		// TODO: Doesn't this the need to add one here and for setupHeightCaches
		//       indicate the caller is treating the request upper bound as inclusive
		//       wrather than exclusive? I don't think it should be needed to add anything
		//       here.
		//this->request<Layer::BiomeBlended>({request.minChunkCoord, request.maxChunkCoord + ChunkVec{1, 1}});
		this->request<Layer::BiomeHeight>({request.minChunkCoord.x, request.maxChunkCoord.x + 1});
		this->request<Layer::BiomeBasis>({request.minChunkCoord, request.maxChunkCoord + ChunkVec{1, 1}});
		generateLayers();

		// TODO: Shrink request based on current stage of chunks. If all chunks, a row, or
		//       a column are already at the final stage we can shrink/skip the request.

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
					(*func)(biomes, terrain, regionCoord, regionIdx, chunkCoord, chunk, /*h0Cache,*/ layerBiomeHeight.cache.cache, std::back_inserter(structures));
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

		// TODO: Consider using a BSP tree, quad tree, BVH, etc. some spatial structure.
		
		// TODO: How do we want to resolve conflicts? If we just go first come first serve
		//       then a spatial  structure isn't needed, you could just check when
		//       inserting. If its not FCFS then we need to keep all boxes and resolve at
		//       the end or else you might over-cull. For example say you have a large
		//       structure that later gets removed by a higher priority small structure.
		//       Since the large structure is now removed there could be other structures
		//       which are now valid since the larger structure no longer exists and
		//       doesn't collide.

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
		const auto& blendStore = layerBiomeBlended.get(chunkCoord);

		// Populate biomes in chunk.
		// TODO: We may want a temporary block granularity biome lookup for structure and decoration generation.
		//       I think there is probably other temp info we would want only during generation also, h0, h1, biome, etc.
		static_assert(CurrentStage != 0, "Stage zero means no generation. This is a bug.");
		if constexpr (CurrentStage == 1) {
			const auto regionBiomeIdx = regionIdxToRegionBiomeIdx(regionIdx);
			for (RegionBiomeUnit x = 0; x < biomesPerChunk; ++x) {
				for (RegionBiomeUnit y = 0; y < biomesPerChunk; ++y) {
					const auto chunkIndex = BlockVec{x, y} * biomesPerChunk;
					region.biomes[regionBiomeIdx.x + x][regionBiomeIdx.y + y] = maxBiomeWeight(blendStore.at(chunkIndex).weights).id;
				}
			}
		}

		// TODO: Find a better solution. This is currently a hack as it isn't populated by
		//       any stage other than the first. We either need to split the first stage
		//       to have its own args or cache the biome info per generate call? Since we
		//       don't need it currently in any of the following stages I'm leaning toward
		//       the first?
		// 
		//       The above is still correct. Info has been moved into the loop, but still
		//       only _needed_ by the first. The layer architecture should help with this.
		//BasisInfo basisInfo{};

		// For each block in the chunk.
		const auto& basisStore = layerBiomeBasis.get(chunkCoord);
		for (BlockUnit x = 0; x < chunkSize.x; ++x) {
			const auto h0 = layerWorldBaseHeight.get(min.x + x);
			for (BlockUnit y = 0; y < chunkSize.y; ++y) {
				const auto chunkIndex = BlockVec{x, y};
				const auto blockCoord = min + chunkIndex;
				const auto basisInfo = basisStore.at(chunkIndex);
				BiomeId biomeId;

				// Generate basis only on the first stage
				if constexpr (CurrentStage == 1) {

					if (basisInfo.basis <= 0.0_f) {
						// TODO: is there a reason we don't just default to air as 0/{}? Do we need BlockId::None?
						chunk.data[x][y] = BlockId::Air;
						continue;
					}
					biomeId = basisInfo.id;
				} else {
					biomeId = maxBiomeWeight(blendStore.at(chunkIndex)).id;
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
	Float Generator<Biomes...>::rm_getHeight1(
		const BiomeId id,
		const BlockUnit blockCoordX,
		const Float h0,
		const BiomeRawInfo2& rawInfo,
		const Float biomeWeight) const {
		BIOME_GEN_DISPATCH_REQUIRED(getHeight, Float, TERRAIN_GET_HEIGHT_ARGS);
		const auto getHeight = BIOME_GET_DISPATCH(getHeight, id);
		return getHeight(biomes, blockCoordX, h0, rawInfo, biomeWeight);
	}

	template<class... Biomes>
	Float Generator<Biomes...>::rm_getBasisStrength(const BiomeId id, const BlockVec blockCoord) const {
		BIOME_GEN_DISPATCH_REQUIRED(getBasisStrength, Float, TERRAIN_GET_BASIS_STRENGTH_ARGS);
		// Output should be between 0 and 1. This is the strength of the basis, not the basis itself.
		const auto getBasisStrength = BIOME_GET_DISPATCH(getBasisStrength, id);
		return getBasisStrength(biomes, blockCoord);
	}
	
	template<class... Biomes>
	Float Generator<Biomes...>::rm_getBasis(const BiomeId id, const BlockVec blockCoord) const {
		BIOME_GEN_DISPATCH_REQUIRED(getBasis, Float, TERRAIN_GET_BASIS_ARGS);
		const auto getBasis = BIOME_GET_DISPATCH(getBasis, id);
		return getBasis(biomes, blockCoord, layerBiomeHeight);
	}
}
