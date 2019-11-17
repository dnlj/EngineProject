#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Noise/Noise.hpp>
#include <Engine/Noise/RangePermutation.hpp>
#include <Engine/Noise/PoissonDistribution.hpp>


namespace Engine::Noise {
	// TODO: move
	// TODO: name?
	template<int32 Value>
	class ConstantDistribution {
		public:
			int32 operator[](const int i) const {
				return Value;
			}
	};

	const static inline auto constant1 = ConstantDistribution<1>{};

	// TODO: Different F_n values as template param? (nth distance)
	// TODO: Split? Inline?
	// TODO: Vectorify?
	// TODO: There seems to be some diag artifacts (s = 0.91) in the noise (existed pre RangePermutation)
	// TODO: For large step sizes (>10ish. very noticeable at 100) we can start to notice repetitions in the noise. I suspect this this correlates with the perm table size.
	// TODO: Do those artifacts show up with simplex as well? - They are. But only for whole numbers? If i do 500.02 instead of 500 they are almost imperceptible.
	template<class Dist>
	class WorleyNoiseGeneric {
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

			// TODO: This code makes some assumptions about `Distribution`. We should probably note those or enforce those somewhere.
			// TODO: make the point distribution a arg or calc in construct
			WorleyNoiseGeneric(int64 seed, const Dist& dist) : perm{seed}, dist{dist} {
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
						const int numPoints = dist[perm.value(cellX, cellY)];

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

		protected:
			RangePermutation<256> perm;
			Dist dist;
	};
	
	// TODO: Doc
	template<auto* Dist>
	class WorleyNoiseFrom : public WorleyNoiseGeneric<decltype(*Dist)> {
		public:
			WorleyNoiseFrom(int64 seed) : WorleyNoiseGeneric{seed, *Dist} {
			}
	};

	// TODO: doc
	class WorleyNoise : public WorleyNoiseFrom<&Distribution::poisson3> {
	};
}
