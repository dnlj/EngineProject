#pragma once

// STD
#include <algorithm>
#include <array>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Noise/Noise.hpp>
#include <Engine/Noise/RangePermutation.hpp>
#include <Engine/Noise/PoissonDistribution.hpp>


// TOOD: Look at "Implementation of Fast and Adaptive Procedural Cellular Noise" http://www.jcgt.org/published/0008/01/02/paper.pdf
namespace Engine::Noise {
	// TODO: move
	// TODO: name?
	template<int32 Value>
	class ConstantDistribution {
		public:
			constexpr int32 operator[](const int i) const {
				return Value;
			}
	};

	const static inline auto constant1 = ConstantDistribution<1>{};

	// TODO: Split? Inline?
	// TODO: Vectorify?
	// TODO: There seems to be some diag artifacts (s = 0.91) in the noise (existed pre RangePermutation)
	// TODO: For large step sizes (>10ish. very noticeable at 100) we can start to notice repetitions in the noise. I suspect this this correlates with the perm table size.
	// TODO: Do those artifacts show up with simplex as well? - They are. But only for whole numbers? If i do 500.02 instead of 500 they are almost imperceptible.
	// TODO: Version/setting for distance type (Euclidean, Manhattan, Chebyshev, Minkowski)
	template<class Dist>
	class WorleyNoiseGeneric {
		public:
			// For easy changing later
			// TODO: template params?
			using Float = float32;
			using Int = int32;

			class Result {
				public:
					/** The x coordinate of the cell the closest point is in. */
					Int x;

					/** The y coordinate of the cell the closest point is in. */
					Int y;

					/** The number of the point in the cell. */
					Int n;

					// TODO: Update comment.
					/** The squared distance from the original input point. */
					Float value = std::numeric_limits<Float>::max();
			};

			// TODO: This code makes some assumptions about `Distribution`. We should probably note those or enforce those somewhere.
			// TODO: make the point distribution a arg or calc in construct
			WorleyNoiseGeneric(int64 seed, const Dist& dist) : perm{seed}, dist{dist} {
			}

			// TODO: doc
			// TODO: name? F1Squared would be more standard
			/**
			 * TODO: finish doc
			 * In this case Result::value is the squared distance to the nearest point.
			 */
			Result valueD2(Float x, Float y) const {
				Result result;

				evaluate(x, y, [&](Float x, Float y, Float px, Float py, Int cx, Int cy, Int ci) ENGINE_INLINE {
					const Float diffX = px - x;
					const Float diffY = py - y;
					const Float d2 = (diffX * diffX) + (diffY * diffY);
					if (d2 < result.value) {
						result = {cx, cy, ci, d2};
					}
				});

				return result;
			}

			// TODO: doc
			// TODO: name
			/**
			 * TODO: finish doc
			 * TODO: typically F2-F1 is with the distances not the squared distances
			 * In this case Result::value is difference between the squared distance to the two nearest point.
			 */
			Result valueF2F1(Float x, Float y) const {
				Result result1;
				Result result2;

				evaluate(x, y, [&](Float x, Float y, Float px, Float py, Int cx, Int cy, Int ci) ENGINE_INLINE {
					const Float diffX = px - x;
					const Float diffY = py - y;
					const Float d2 = (diffX * diffX) + (diffY * diffY);
					if (d2 < result1.value) {
						result2 = result1;
						result1 = {cx, cy, ci, d2};
					} else if (d2 < result2.value) {
						result2 = {cx, cy, ci, d2};
					}
				});

				result1.value = result2.value - result1.value;
				return result1;
			}

		protected:
			RangePermutation<256> perm;
			Dist dist;

			// TODO: Doc
			template<class PointProcessor>
			void evaluate(const Float x, const Float y, PointProcessor&& pp) const {
				// Figure out which base unit square we are in
				Int baseX = floorTo<Int>(x);
				Int baseY = floorTo<Int>(y);

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
							pp(x, y, pointX, pointY, cellX, cellY, i);
						}
					}
				}
			}
	};
	
	// TODO: Doc
	template<auto* Dist>
	class WorleyNoiseFrom : public WorleyNoiseGeneric<decltype(*Dist)> {
		public:
			WorleyNoiseFrom(int64 seed) : WorleyNoiseGeneric<decltype(*Dist)>{seed, *Dist} {
			}
	};

	// TODO: doc
	class WorleyNoise : public WorleyNoiseFrom<&poisson3> {
	};
}
