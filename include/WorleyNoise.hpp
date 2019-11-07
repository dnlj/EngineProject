#pragma once

// STD
#include <cstdint>

// TODO: For large step sizes (>10ish. very noticeable at 100) we can start to notice repetitions in the noise. I suspect this this correlates with the perm table size.
// TODO: Do those artifacts show up with simplex as well? - They are. But only for whole numbers? If i do 500.02 instead of 500 they are almost imperceptible.
class WorleyNoise {
	public:
		// TODO: move into Engine and specifys all types. float32, float64, int32, uint32 etc. Make sure to verify behavior/size. May conflict with box2d? Can use numeric_limits for asserts. http://www.cs.technion.ac.il/users/yechiel/c++-faq/bytes-review.html
		// TODO: Verify that a float is 32bits and a specific type. Static asserts
		using float32 = float;

		// For easy changing later
		// TODO: template params?
		using Float = float32;
		using Int = int32_t;

		// TODO: Does an unsigned seed change anything?
		WorleyNoise(int64_t seed) { // TODO: replace. Stolen from OpenSimplexNoise.
			// Generate a source array with values [0, 255]
			decltype(perm) source;
			for (int i = 0; i < 256; i++) {
				source[i] = i & 0xFF;
			}

			// LCG RNG
			seed = seed * 6364136223846793005l + 1442695040888963407l;
			seed = seed * 6364136223846793005l + 1442695040888963407l;
			seed = seed * 6364136223846793005l + 1442695040888963407l;

			// Populate perm
			for (int i = 255; i >= 0; i--) {
				seed = seed * 6364136223846793005l + 1442695040888963407l;

				// TODO: Why the "+ 31"?
				// Limit the random number to [-i, i]
				int r = (int)((seed + 31) % (i + 1));

				// If the number is < 0 add i + 1 giving a range of [0, i]
				if (r < 0) {
					r += (i + 1);
				}

				// Populate perm with the value of source[r]
				perm[i] = source[r];

				// Since we will never visit source[i] again, move the value of source[i] into source[r] so we dont miss any values. (because range is limited to [0, i] where i is decreasing)
				source[r] = source[i];
			}
		}

		// TODO: name?
		Float at(Float x, Float y) {
			return index(fastFloor(x), fastFloor(y)) / 255.0f;
		}

	private:
		// TODO: Check if this is actually faster
		constexpr static Int fastFloor(Float x) {
			Int xi = static_cast<Int>(x);
			return x < xi ? xi - 1 : xi;
		}

		constexpr int8_t index(Int x) const {
			return perm[x & 0xFF];
		}

		constexpr int8_t index(Int x, Int y) const {
			return perm[(index(x) + y) & 0xFF];
		}

	private:
		/** Stores all numbers [0, 255] in a random order based on the initial seed */
		// TODO: Compute this at runtime like open simplex does. Could also populate it from a seeded constexpr function.
		uint8_t perm[256] = {}; // TODO: When move into lib, have a shared perm array?
};
