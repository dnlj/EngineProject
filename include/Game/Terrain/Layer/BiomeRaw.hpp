#pragma once

#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.

namespace Game::Terrain::Layer {
	// The direct, raw, biome info. Determines what biome is where before any blending/interpolation.
	class BiomeRaw : public DependsOn<> {
		public:
			using Range = ChunkArea;
			using Index = BlockVec;

		public:
			BiomeRaw(uint64 seed)
				: biomeFreq{seed}
				, biomePerm{Engine::Noise::lcg(seed)}
			{}

			void request(const Range area, TestGenerator& generator);
			void generate(const Range blockCoord, TestGenerator& generator);
			[[nodiscard]] BiomeRawInfo2 get(const Index blockCoord) const noexcept;

		private:
			// TODO: This isn't great. This has a repeat every X biomes. This can be
			//       easily seen in the terrain preview by going to BiomeBaseGrid mode and
			//       then jumping between X = 0 to X = PermSize * SmallBiomeSize. Notice
			//       it is almost identical. It won't be exactly identical because we have
			//       three octaves of biomes at different sizes. So each octaves will
			//       repeat at a different scale, but it would be very noticable in
			//       gameplay. We should be able to significantly improve this range by
			//       using a hash-like function instead of a permutation table.
			//       Related: Qn46mY7Y
			//
			/** Used for sampling the biome frequency. */
			Engine::Noise::RangePermutation<256> biomeFreq;

			// TODO: We need different sample overloads, we want the result unit for this to be
			//       BiomeType, but we want to sample with BlockUnit.
			/** Used for sampling the biome type. */
			Engine::Noise::RangePermutation<256> biomePerm;

			// TODO: re-enable this somewhere. Here isn't a good place anymore since we have reduced the dependencies.
			//static_assert(sizeof...(Biomes) <= decltype(biomePerm)::size());
	};
}
