#pragma once

// Engine
#include <Engine/Noise/noise.hpp>
#include <Engine/Engine.hpp>


namespace Engine::Noise {
	template<int32 Size, std::integral Int = int32> // TODO: min, max
	class RangePermutation {
		public:
			using Stored = uint8;

		private:
			static_assert(Size > 0, "Size must be non-negative.");
			static_assert(Size <= 256, "Values are currently stored as uint8.");
			Stored perm[Size];

			constexpr static bool isPowerOfTwo = Size && !(Size & (Size - 1));

		public:
			RangePermutation(int64 seed) {
				// Generate a source array with values [0, Size - 1]
				decltype(perm) source;
				for (Int i = 0; i < Size; i++) {
					source[i] = static_cast<Stored>(i);
				}

				// TODO: is this needed? (originally from OpenSimplexNoise)
				// LCG RNG
				seed = lcg(seed);
				seed = lcg(seed);
				seed = lcg(seed);

				// Populate perm
				for (Int i = Size - 1; i >= 0; i--) {
					seed = lcg(seed);

					// Ensure r is positive. We can't use abs because abs(-INT_MAX) = -INT_MAX due to overflow
					Int r = seed & 0x7FFFFFFF;

					// TODO: Why the "+ 31"? (originally from OpenSimplexNoise)
					// Limit r to [0, i]. Using % can introduce bias.
					r = (Int)((r + 31) % (i + 1));

					// Populate perm with the value of source[r]
					perm[i] = source[r];

					// Since we will never visit source[i] again, move the value of source[i] into source[r] so we dont miss any values. (because range is limited to [0, i] where i is decreasing)
					source[r] = source[i];
				}
			}

			template<class... Args>
			ENGINE_INLINE Int operator()(Args... args) const { return value(args...); }

			ENGINE_INLINE Int value(Int x) const {
				if constexpr (isPowerOfTwo) {
					// Some reason this isnt automatically done by the compiler
					return perm[x & (Size - 1)];
				} else {
					// TODO: look into fastmod: https://www.youtube.com/watch?v=nXaxk27zwlk&t=56m34s
					return perm[x % Size];
				}
			}

			ENGINE_INLINE Int value(Int x, Int y) const {
				return value(value(x) + y);
			}

			ENGINE_INLINE Int value(Int x, Int y, Int z) const {
				return value(value(x, y) + z);
			}
	};
}
