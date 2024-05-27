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
	template<StageId TotalStages, class... Biomes>
	void Generator<TotalStages, Biomes...>::generate1(Terrain& terrain, const Request& request) {
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

		//(generate<Biomes>(terrain, request), ...);

		// Call generate for each stage. Each will expand the requestion chunk selection
		// appropriately for the following stages.
		Engine::Meta::ForEachInRange<TotalStages>::call([&]<auto I>{
			// +1 because stage zero = uninitialized, zero stages have been run yet.
			generate<I+1>(terrain, request);
		});
	}

	template<StageId TotalStages, class... Biomes>
	template<StageId CurrentStage>
	void Generator<TotalStages, Biomes...>::generate(Terrain& terrain, const Request& request) {
		constexpr ChunkUnit offset = TotalStages - CurrentStage;
		const auto min = request.minChunkCoord - offset;
		const auto max = request.maxChunkCoord + offset;

		for (auto cur = min; cur.x <= max.x; ++cur.x) {
			for (cur.y = min.y; cur.y <= max.y; ++cur.y) {
				auto& region = terrain.getRegion({request.realmId, chunkToRegion(cur)});
				auto& stage = region.stages[cur.x][cur.y];

				if (stage < CurrentStage) {
					ENGINE_DEBUG_ASSERT(stage == CurrentStage - 1);
					generateChunk<CurrentStage>(terrain, cur, region.chunks[cur.x][cur.y]);
					++stage;
				}
			}
		}
	}

	template<StageId TotalStages, class... Biomes>
	template<StageId CurrentStage>
	void Generator<TotalStages, Biomes...>::generateChunk(Terrain& terrain, const ChunkVec chunkCoord, Chunk& chunk) {
		// If there is a compilation error here your stage<#> overload is likely missing
		// `static` or has incorrect arguments.
		using BiomeStageFunc = void (*)(void* self, Terrain& terrain, const ChunkVec chunkCoord, Chunk& chunk, const BiomeInfo biomeInfo);
		constexpr static BiomeStageFunc stageForBiome[] = { &Biomes::template stage<CurrentStage>... };


		// TODO: finish comment
		// We need to do the biome stages this way because of the artificial limitation
		// C++ imposes that no template function, _including fully specialized template
		// functions_, cannot be virtual. Since they can't be specialized and virtual we
		// have no way to pass the current stage other than to resolve it at runtime,
		// which would involve at minimum an additional ptr chase and two indexes from the
		// vtable and stage resolution. We know... TODO: fin

		const auto min = chunkToBlock(chunkCoord);
		const auto max = min + chunkSize;
		
		for (auto cur = min; cur.x < max.x; ++cur.x) {
			for (cur.y = min.y; cur.y < max.y; ++cur.y) {
				// TODO: One thing to consider is that we loose precision when converting
				//       from BlockCoord to FVec2. Not sure how to solve that other than use
				//       doubles, and that will be slower and still isn't perfect.

				//
				//
				//
				//
				// TODO: can we wrap this in a funciton that does the
				//       `static_cast<Biome*>->stage<N>` and then force inline the stages?
				//       Then the biomes appear to be normal and there should be no
				//       additional overhead if things _actually_ get inlined. That should
				//       be doable with forceinline_calls so we don't have to remember on
				//       each individual function, but we need to check that that actually
				//       works, i think last I looked it didn't seem to? Does it trigger
				//       the warning we have enabled? If not is there a separate warning?
				//
				//
				//
				//
				//
				//

				const auto biomeInfo = calcBiome(cur);
				//ENGINE_LOG2("Biome: {}", +biomeInfo.type);
				stageForBiome[+biomeInfo.type](biomesErased[+biomeInfo.type], terrain, cur, chunk, biomeInfo);
			}
		}
	}

	template<StageId TotalStages, class... Biomes>
	BiomeInfo Generator<TotalStages, Biomes...>::calcBiome(const BlockVec blockCoord) {
		// TODO: if we simd-ified this we could do all scale checks in a single pass.

		// TODO: Force/manual unroll? We know that N is always small and that would avoid
		//       a scale lookup. This function gets called _a lot_.
		BiomeInfo result{};

		// Determine the scale and sample cell for the biome.
		// Check each scale. Must be ordered largest to smallest.
		while (true) {
			// Rescale the coord so that all scales sample from the same mapping.
			// Originally: cell = glm::floor(blockCoord * biomeScalesInv[result.scale]);
			result.cell = Engine::Math::divFloor(blockCoord, biomeScales[+result.scale]).q;

			// TODO: doc - I think 10 was an arbitrary num here? I believe this controls how
			//       frequently each scale is selected? Should be able to use a different number
			//       for each scale if we want different frequencies.
			if (perm(result.cell.x, result.cell.y) < 10) { break; }

			// No more scales to check. Don't increment so we use the smallest one.
			if (result.scale == BiomeScale::_last) { break; }
			++result.scale;
		}

		// TODO: something is broken here:
		//result.type = static_cast<BiomeType>(biomeTypePerm(result.cell.x, result.cell.y));
		//result.type = static_cast<BiomeType>(result.cell.x % sizeof...(Biomes));
		result.type = static_cast<BiomeType>((blockCoord.x / 64) % sizeof...(Biomes));
		return result;
	}
}
