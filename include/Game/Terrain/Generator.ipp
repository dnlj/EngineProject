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
		// TODO: Move/redocument things in terms of layers once transition is done.
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
		//       rather than exclusive? I don't think it should be needed to add anything
		//       here.
		const Layer::ChunkArea chunkArea = {request.minChunkCoord, request.maxChunkCoord + ChunkVec{1, 1}};
		this->request<Layer::BiomeHeight>({request.minChunkCoord.x, request.maxChunkCoord.x + 1});
		this->request<Layer::BiomeBlock>(chunkArea);
		this->request<Layer::BiomeStructures>(chunkArea);
		generateLayers();

		// TODO: Update comment. Stages have been removed.
		// Call generate for each stage. Each will expand the requestion chunk selection
		// appropriately for the following stages.
		for (auto chunkCoord = request.minChunkCoord; chunkCoord.x <= request.maxChunkCoord.x; ++chunkCoord.x) {
			for (chunkCoord.y = request.minChunkCoord.y; chunkCoord.y <= request.maxChunkCoord.y; ++chunkCoord.y) {
				const auto regionCoord = chunkToRegion(chunkCoord);
				auto& region = terrain.getRegion({request.realmId, regionCoord});
				const auto regionIdx = chunkToRegionIndex(chunkCoord, regionCoord);
				auto& populated = region.populated[regionIdx.x][regionIdx.y];
				
				if (!populated) {
					// For each block in the chunk.
					// TODO: Once layers are done this loop should go away. Can just access the
					//       layerBiomeBlock directly.
					const auto chunkBiomeBlock = layerBiomeBlock.get(chunkCoord);
					auto& chunk = region.chunkAt(regionIdx);
					for (BlockUnit x = 0; x < chunkSize.x; ++x) {
						for (BlockUnit y = 0; y < chunkSize.y; ++y) {
							chunk.data[x][y] = chunkBiomeBlock.data[x][y];
						}
					}

					populated = true;
				}
			}
		}

		layerBiomeStructures.get(chunkArea, *this, request.realmId, terrain);
	}

	template<class... Biomes>
	Float Generator<Biomes...>::rm_getBasis(const BiomeId id, const BlockVec blockCoord) const {
		BIOME_GEN_DISPATCH_REQUIRED(getBasis, Float, TERRAIN_GET_BASIS_ARGS);
		const auto getBasis = BIOME_GET_DISPATCH(getBasis, id);
		return getBasis(biomes, blockCoord, layerBiomeHeight);
	}

	template<class... Biomes>
	BlockId Generator<Biomes...>::rm_getStage(const BiomeId id, const BlockVec blockCoord, const BasisInfo& basisInfo) const {
		BIOME_GEN_DISPATCH_T(stage, stage<1>, BlockId, TERRAIN_STAGE_ARGS);
		const auto getStage = BIOME_GET_DISPATCH(stage, id);
		return getStage(biomes, blockCoord, basisInfo);
	}

	template<class... Biomes>
	void Generator<Biomes...>::rm_getStructureInfo(const BiomeId id, const ChunkVec chunkCoord, std::back_insert_iterator<std::vector<StructureInfo>> inserter) {
		BIOME_GEN_DISPATCH(getLandmarks, void, TERRAIN_GET_LANDMARKS_ARGS);
		const auto func = BIOME_GET_DISPATCH(getLandmarks, id);

		if (func) {
			(*func)(biomes, chunkCoord, layerBiomeHeight.cache.cache, inserter);
		}
	}

	template<class... Biomes>
	void Generator<Biomes...>::rm_getStructures(const StructureInfo& info, const RealmId realmId, Terrain& terrain) {
		BIOME_GEN_DISPATCH(genLandmarks, void, TERRAIN_GEN_LANDMARKS_ARGS);
		const auto func = BIOME_GET_DISPATCH(genLandmarks, info.biomeId);
		ENGINE_DEBUG_ASSERT(func, "Attempting to generate landmark in biome that does not support them.");
		func(biomes, terrain, realmId, info);
	}
}
