#pragma once

// Engine
#include <Engine/Noise/Noise.hpp>
#include <Engine/Engine.hpp>


namespace Engine::Noise {
	template<int32 Size> // TODO: min, max
	class RangePermutation {
		static_assert(Size > 0, "Size must be non-negative.");
		constexpr static bool isPowerOfTwo = Size && !(Size & (Size - 1));

		public:
			RangePermutation(int64 seed) {
				// Generate a source array with values [0, Size - 1]
				decltype(perm) source;
				for (int i = 0; i < Size; i++) {
					source[i] = i;
				}

				// TODO: is this needed? (originally from OpenSimplexNoise)
				// LCG RNG
				seed = lcg(seed);
				seed = lcg(seed);
				seed = lcg(seed);

				// Populate perm
				for (int i = Size - 1; i >= 0; i--) {
					seed = lcg(seed);

					// Ensure r is positive. We can't use abs because abs(-INT_MAX) = -INT_MAX due to overflow
					int32 r = seed & 0x7FFFFFFF;

					// TODO: Why the "+ 31"? (originally from OpenSimplexNoise)
					// Limit r to [0, i]. Using % can introduce bias.
					r = (int)((r + 31) % (i + 1));

					// Populate perm with the value of source[r]
					perm[i] = source[r];

					// Since we will never visit source[i] again, move the value of source[i] into source[r] so we dont miss any values. (because range is limited to [0, i] where i is decreasing)
					source[r] = source[i];
				}
			}

			ENGINE_INLINE int32 value(int32 x) const {
				if constexpr (isPowerOfTwo) {
					// Some reason this isnt automatically done by the compiler
					return perm[x & (Size - 1)];
				} else {
					// TODO: look into fastmod: https://www.youtube.com/watch?v=nXaxk27zwlk&t=56m34s
					return perm[x % Size];
				}
			}

			ENGINE_INLINE int32 value(int32 x, int32 y) const {
				return value(value(x) + y);
			}

			ENGINE_INLINE int32 value(int32 x, int32 y, int32 z) const {
				return value(value(x, y) + z);
			}

		private:
			uint8 perm[Size];
	};
}
