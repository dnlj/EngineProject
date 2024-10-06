#pragma once

// Engine
#include <Engine/Noise/noise.hpp>
#include <Engine/Engine.hpp>


namespace Engine::Noise {
	template<uint32 Size> // TODO: min, max
	class RangePermutation {
		using Stored = uint8;
		static_assert(std::is_unsigned_v<Stored>, "The stored type must be unsigned.");

		public:
			using StoredUnderlying = Engine::AsIntegral<Stored>::type;

		private:
			static_assert(Size > 0, "Size must be non-negative.");
			static_assert((Size - 1) <= std::numeric_limits<StoredUnderlying>::max(), "Values are currently stored as uint8.");
			Stored perm[Size];

			constexpr static bool isPowerOfTwo = Size && !(Size & (Size - 1));

		public:
			constexpr RangePermutation(uint64 seed) noexcept {
				using Index = int32;

				// Generate a source array with values [0, Size - 1]
				decltype(perm) source;
				for (Index i = 0; i < Size; i++) {
					source[i] = static_cast<Stored>(i);
				}

				// TODO: is this needed? (originally from OpenSimplexNoise)
				// LCG RNG
				seed = lcg(seed);
				seed = lcg(seed);
				seed = lcg(seed);

				// Populate perm
				for (Index i = Size - 1; i >= 0; i--) {
					seed = lcg(seed);

					// Ensure r is positive. We can't use abs because abs(-INT_MAX) = -INT_MAX due to overflow
					Index r = seed & 0x7FFFFFFF;

					// TODO: Why the "+ 31"? (originally from OpenSimplexNoise)
					// Limit r to [0, i]. Using % can introduce bias.
					r = (Index)((r + 31) % (i + 1));

					// Populate perm with the value of source[r]
					perm[i] = source[r];

					// Since we will never visit source[i] again, move the value of source[i] into source[r] so we dont miss any values. (because range is limited to [0, i] where i is decreasing)
					source[r] = source[i];
				}
			}

			constexpr static decltype(Size) size() noexcept { return Size; }

			template<class... Args>
			ENGINE_INLINE constexpr Stored operator()(Args... args) const noexcept {
				// The static cast is just here for compatibility with existing code where
				// before we had separate index and stored types. Having separate index types
				// doesn't make sense because we are going to modulo the index anyways.
				return value(static_cast<Stored>(args)...);
			}

			ENGINE_INLINE constexpr Stored value(Stored x) const noexcept {
				if constexpr (isPowerOfTwo) {
					// Some reason this _is not_ automatically done by the compiler
					return perm[x & (Size - 1)];
				} else {
					// TODO: look into fastmod: https://www.youtube.com/watch?v=nXaxk27zwlk&t=56m34s
					return perm[x % Size];
				}
			}

			// TODO: Is there a better way to index of multiple dimension than calling
			//       `value` multiple times? Is that significant to the function of this?
			//       I don't think so? Need to refresh my memory on how this works.
			//       Indexing multiple times into the perm array probably isn't ideal
			//       since this is a known significant (measured) bottleneck.
			ENGINE_INLINE constexpr Stored value(Stored x, Stored y) const noexcept {
				return value(value(x) + y);
			}

			ENGINE_INLINE constexpr Stored value(Stored x, Stored y, Stored z) const noexcept {
				return value(value(x, y) + z);
			}
	};
}
