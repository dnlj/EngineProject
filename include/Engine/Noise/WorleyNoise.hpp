#pragma once

// STD
#include <cstdint>
#include <cmath>

// GLM
#include <glm/glm.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Noise/Noise.hpp>


namespace Engine::Noise {
	// TODO: Different F_n values as template param? (nth distance)
	// TODO: Split? Inline?
	// TODO: Vectorify?
	// TODO: For large step sizes (>10ish. very noticeable at 100) we can start to notice repetitions in the noise. I suspect this this correlates with the perm table size.
	// TODO: Do those artifacts show up with simplex as well? - They are. But only for whole numbers? If i do 500.02 instead of 500 they are almost imperceptible.
	class WorleyNoise {
		public:
			// For easy changing later
			// TODO: template params?
			using Float = float32;
			using Int = int32;

			// TODO: make the point distribution a arg or calc in construct
			WorleyNoise(int64 seed) { // TODO: replace. Stolen from OpenSimplexNoise.
				// Generate a source array with values [0, 255]
				decltype(perm) source;
				for (int i = 0; i < 256; i++) {
					source[i] = i & 0xFF;
				}

				// LCG RNG
				seed = lcg(seed);
				seed = lcg(seed);
				seed = lcg(seed);

				// Populate perm
				for (int i = 255; i >= 0; i--) {
					seed = lcg(seed);

					// TODO: Why the "+ 31"?
					// Limit the random number to [-i, i]
					int r = (int)((seed + 31) % (i + 1));

					// TODO: is there any reason to do this over `r = abs(r)` ???
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

			// TODO: Doc
			// TODO: name?
			Float value(const Float x, const Float y) {
				// Figure out which base unit square we are in
				const Int baseX = floorTo<Int>(x);
				const Int baseY = floorTo<Int>(y);
				Float minDistSquared = std::numeric_limits<Float>::max();

				// TODO: check boundary cubes. Based on our closest point we can cull rows/cols		
				for (int offsetY = -1; offsetY < 2; ++offsetY) {
					for (int offsetX = -1; offsetX < 2; ++offsetX) {
						// Position and points in this cell
						const Int cellX = baseX + offsetX;
						const Int cellY = baseY + offsetY;
						const int numPoints = poisson[index(cellX, cellY)];

						// Find the smallest squared distance in this cell
						for (int i = 0; i < numPoints; ++i) {
							const Float pointX = cellX + index(cellX, cellY, +i) / Float{255};
							const Float pointY = cellY + index(cellX, cellY, -i) / Float{255};
							const Float diffX = pointX - x;
							const Float diffY = pointY - y;
							const Float distSquared = (diffX * diffX) + (diffY * diffY);

							if (distSquared < minDistSquared) {
								minDistSquared = distSquared;
							}
						}
					}
				}

				// Return the true distance
				return std::sqrt(minDistSquared);
			}

		private:
			constexpr Int index(const Int x) const {
				return perm[x & 0xFF];
			}

			constexpr Int index(const Int x, const Int y) const {
				return perm[(index(x) + y) & 0xFF];
			}

			constexpr Int index(const Int x, const Int y, const Int z) const {
				return perm[(index(x, y) + z) & 0xFF];
			}

		private:
			/** Stores all numbers [0, 255] in a random order based on the initial seed */
			uint8 perm[256] = {};

			// TODO: do we want to clamp this range to something like [1, 9] like suggested in the paper?
			// TODO: Make a constexpr function to generate this array for any given mean
			/** Perfect Poisson distribution with mean = 4 for 256 values in random order */
			const int8 poisson[256] = {
				 2,  1,  4,  7,  3,  2,  3,  5,  5,  5,  0,  4,  7,  2,  3,  4,  9,  4,  3,  2,  6,  5,  5,  3,  4,  4,  4, 10,  4,  5,  6,  3,
				 2,  1,  5,  2,  2,  1,  6,  6,  6,  7,  4,  5,  6,  2,  6,  4,  6,  3,  5,  7,  2,  4,  3,  3,  3,  8,  5,  7,  4,  3,  1,  6,
				 3,  3,  4,  3,  5,  3,  3,  5,  1,  6,  3,  3,  2,  3,  6,  2,  4,  5,  4,  3,  4,  7,  4,  3,  4,  6,  3,  9,  4,  8,  6,  2,
				 4,  3,  4,  6,  7,  3,  5,  4,  5,  5,  1,  7,  2,  4,  5,  1,  0,  2,  5,  2,  4,  3,  3,  4,  3,  2,  5,  5,  2,  4,  5,  8,
				 2,  4,  4,  3,  4,  5,  7,  3,  4,  2,  1,  4,  4,  0,  5,  6,  7,  8,  5,  3,  3,  1,  4,  4,  5,  3,  6,  4,  6,  6,  4,  4,
				 3,  5,  1,  4,  3,  4,  4,  5,  2,  2,  2,  3,  5,  7,  1,  6,  2,  8,  3,  1,  4,  3,  1,  1,  1,  2,  4,  4,  3,  2,  3,  6,
				 5,  8,  2,  6,  5,  2,  5,  6,  1,  3,  6,  1,  8,  2,  3,  6,  0,  2,  3,  6,  5,  4,  3,  7,  1,  9,  4,  5,  3,  5,  5,  2,
				 2,  5,  0,  3,  7,  3,  2,  3,  4,  5,  2,  4,  7,  6,  4,  4,  2,  7,  2,  5,  8,  1,  2,  2,  6,  3,  3,  3,  5,  2,  5,  4,
			};

			/** The maximum value in #poisson */
			constexpr static int32 POISSON_MAX = 10;
	};
}
