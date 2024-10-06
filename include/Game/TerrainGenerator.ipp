#include <Game/TerrainGenerator.hpp>

// TODO: can't do this in ipp file, not sure how this would work exactly, but it would
//       avoid allocating and arguably some lookup in the middle of generation.
// 
//namespace {
//	class RequestCache {
//		public:
//			Region& topLeft;
//			Region& topCenter;
//			Region& topRight;
//			Region& midLeft;
//			Region& midCenter;
//			Region& midRight;
//			Region& botLeft;
//			Region& botCenter;
//			Region& botRight;
//	};
//}

namespace Game::Terrain {
	template<class... Biomes>
	void Generator<Biomes...>::generate1(Terrain& terrain, const Request& request) {
		//auto& region = terrain.getRegion(chunkCoord.toRegion());


		// Need a way to optimize this process so we don't go back and forth constantly:
		// - everything to stage 1
		// - everything to stage 2
		// - everything to stage 3
		// - etc.
		// - each stage will need to shrink though, so based on the requested chunks we
		//   need to figure out how far out we need to go.
		// 
		// for each chunk in stack:
		//   for each neighbor of chunk:
		//     if neighbor is not at correct stage:
		//       add neighbor to stack;

		// Call generate for each stage. Each will expand the requestion chunk selection
		// appropriately for the following stages.
		Engine::Meta::ForEachInRange<totalStages>::call([&]<auto I>{
			// +1 because stage zero = uninitialized, zero stages have been run yet.
			generate<I+1>(terrain, request);
		});
	}

	template<class... Biomes>
	template<StageId CurrentStage>
	void Generator<Biomes...>::generate(Terrain& terrain, const Request& request) {
		constexpr ChunkUnit offset = totalStages - CurrentStage;
		const auto min = request.minChunkCoord - offset;
		const auto max = request.maxChunkCoord + offset;

		//
		// TODO: Feature and decoration generation.
		// 
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
		//

		// For each chunk in the request
		for (auto cur = min; cur.x <= max.x; ++cur.x) {
			for (cur.y = min.y; cur.y <= max.y; ++cur.y) {
				auto& region = terrain.getRegion({request.realmId, chunkToRegion(cur)});
				const auto chunkIndex = chunkToRegionIndex(cur);
				auto& stage = region.stages[chunkIndex.x][chunkIndex.y];

				if (stage < CurrentStage) {
					ENGINE_DEBUG_ASSERT(stage == CurrentStage - 1);
					generateChunk<CurrentStage>(terrain, cur, region.chunks[chunkIndex.x][chunkIndex.y]);
					ENGINE_LOG2("Generate Chunk: {}", cur);
					++stage;
				}
			}
		}
	}
	
	template<class... Biomes>
	template<StageId CurrentStage, class Biome>
	ENGINE_INLINE_REL BlockId Generator<Biomes...>::callStage(TERRAIN_STAGE_ARGS) {
		// Force inline the calls to avoid the extra function call overhead. These are exactly
		// 1:1 so there is no reason to not do this. Originally we passed a `void* self` and
		// biomes had to recast back to the correct type to use it. Doing things like we do
		// below instead allows the biomes to act like normal classes and use `this` in the
		// stage functions. This is ultimately just for convenience and a continuation of the
		// workaround that we can't overload template specializations (See generateChunk for
		// details).

		// On MSVC [[msvc::forceinline_calls]] doesn't seem to work 100% of the time and
		// gives no warning, such as C4714, if it fails. As such all stage functions
		// should be marked with ENGINE_INLINE also.
		ENGINE_INLINE_CALLS {
			return std::get<Biome>(biomes).stage<CurrentStage>(terrain, chunkCoord, blockCoord, blockIndex, chunk, biomeInfo, h0);
		}
	}

	template<class... Biomes>
	template<StageId CurrentStage>
	void Generator<Biomes...>::generateChunk(Terrain& terrain, const ChunkVec chunkCoord, Chunk& chunk) {
		// We need to wrap this in a function to avoid instantiating stages that don't
		// exist since some biomes will not define all stages.
		constexpr static auto getStageCall = []<class Biome>() {
			if constexpr (HasStage<Biome, CurrentStage>) {
				return &Generator::template callStage<CurrentStage, Biome>;
			} else {
				return nullptr;
			}
		};

		// TODO: Do we need to pass terrain/chunk? They should probably at least be const. Remove if not used, not sure why we would need chunk here.
		using BiomeStageFunc = BlockId (Generator::*)(TERRAIN_STAGE_ARGS);
		constexpr static BiomeStageFunc stageForBiome[] = { getStageCall.template operator()<Biomes>()... };
		constexpr static bool hasStage[] = { HasStage<Biomes, CurrentStage>... };

		// TODO: finish comment
		// We need to do the biome stages this way (via func ptr) because of the
		// artificial limitation C++ imposes that no template function, _including fully
		// specialized template functions_, cannot be virtual. Since they can't be
		// specialized and virtual we have no way to pass the current stage other than to
		// resolve it at runtime, which would involve at minimum an additional ptr chase
		// and two indexes from the vtable and stage resolution. We know... TODO: fin

		const auto min = chunkToBlock(chunkCoord);
		const auto max = min + chunkSize;

		// TODO: would it be beneficial to generate all biome dat first? as a stage 0?
		// TODO: Does a switch work better here for biome? i would assume its the same

		// For each block in the chunk
		for (BlockUnit x = 0; x < chunkSize.x; ++x) {
			for (BlockUnit y = 0; y < chunkSize.y; ++y) {
				const auto blockCoord = min + BlockVec{x, y};
				const auto biomeInfo = calcBiome(blockCoord);

				if (hasStage[biomeInfo.id]) {
					const auto func = stageForBiome[biomeInfo.id];
					const auto blockId = (this->*func)(terrain, chunkCoord, blockCoord, {x, y}, chunk, biomeInfo, 0);
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
