#pragma once

// STD
#include <cstdint>
#include <cmath>

// GLM
#include <glm/glm.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Noise/Noise.hpp>
#include <Engine/Noise/RangePermutation.hpp>


namespace Engine::Noise {
	// TODO: Different F_n values as template param? (nth distance)
	// TODO: Split? Inline?
	// TODO: Vectorify?
	// TODO: There seems to be some diag artifacts (s = 0.91) in the noise (existed pre RangePermutation)
	// TODO: For large step sizes (>10ish. very noticeable at 100) we can start to notice repetitions in the noise. I suspect this this correlates with the perm table size.
	// TODO: Do those artifacts show up with simplex as well? - They are. But only for whole numbers? If i do 500.02 instead of 500 they are almost imperceptible.
	class WorleyNoise {
		public:
			// For easy changing later
			// TODO: template params?
			using Float = float32;
			using Int = int32;

			struct Result {
				/** The x coordinate of the cell the closest point is in. */
				Int x;

				/** The y coordinate of the cell the closest point is in. */
				Int y;

				/** The number of the point in the cell. */
				Int n;

				/** The squared distance from the original input point. */
				Float distanceSquared = std::numeric_limits<Float>::max();
			};

			// TODO: make the point distribution a arg or calc in construct
			WorleyNoise(int64 seed) : perm{seed} {
			}

			// TODO: Doc
			// TODO: name?
			Result value(const Float x, const Float y) {
				// Figure out which base unit square we are in
				Int baseX = floorTo<Int>(x);
				Int baseY = floorTo<Int>(y);
				Result result;

				// TODO: check boundary cubes. Based on our closest point we can cull rows/cols
				for (int offsetY = -1; offsetY < 2; ++offsetY) {
					for (int offsetX = -1; offsetX < 2; ++offsetX) {
						// Position and points in this cell
						const Int cellX = baseX + offsetX;
						const Int cellY = baseY + offsetY;
						const int numPoints = poisson[perm.value(cellX, cellY)];

						// Find the smallest squared distance in this cell
						for (int i = 0; i < numPoints; ++i) {
							const Float pointX = cellX + perm.value(cellX, cellY, +i) / Float{255};
							const Float pointY = cellY + perm.value(cellX, cellY, -i) / Float{255};
							const Float diffX = pointX - x;
							const Float diffY = pointY - y;
							const Float distSquared = (diffX * diffX) + (diffY * diffY);

							if (distSquared < result.distanceSquared) {
								result.x = cellX;
								result.y = cellY;
								result.n = i;
								result.distanceSquared = distSquared;
							}
						}
					}
				}

				// Return the true distance
				return result;
			}

		private:
			RangePermutation<256> perm;

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
