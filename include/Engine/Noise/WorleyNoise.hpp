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
	// TODO: just make this a function?
	template<int Count, int Mean, int Min, int Max>
	class PoissonDistribution {
		static_assert(Min < Mean);
		static_assert(Max > Mean);
		public:
			PoissonDistribution() {
				std::cout << "PoissonDistribution<" << Count << ", " << Mean << ", " << Min << ", " << Max << ">\n";
				constexpr int size = Max - Min;
				float64 percent[size];
				float64 total = 0.0f;
				const float64 logMean = log(Mean);

				for (int i = 0; i < size; ++i) {
					// Find log of pmf
					const auto k = Min + i;
					float64 pmf = k * logMean - Mean;

					// ... / ln(k!) = ... - ln(k) - ln(k - 1) - ... - ln(2)
					for (int j = k; j > 1; --j) {
						pmf -= log(j);
					}

					// Get true value of pmf
					pmf = exp(pmf);
					std::cout << "PMF(" << k << "; " << Mean << ") = " << pmf << "\n";
					percent[i] = pmf;
					total += pmf;
				}

				// Distribute the values from most probable to least.
				int dataOffset = 0;
				int lo = Mean;
				int hi = Mean + 1;
				constexpr int loBound = Min - 1;
				constexpr int hiBound = Max;
				int remaining = Count;
				while(lo > loBound || hi < hiBound) {
					int cur;

					if (hi == hiBound || (lo != loBound && percent[lo] > percent[hi])) {
						cur = lo;
						--lo;
					} else {
						cur = hi;
						++hi;
					}

					// Get the number of `cur`s to insert
					const auto p = percent[cur - Min];
					int amount = static_cast<int>(round(p / total * remaining));
					total -= p;
					remaining -= amount;

					// Populate data
					const auto dataP = data.data() + dataOffset;
					std::fill(dataP, dataP + amount, cur);
					dataOffset += amount;
				}

				ENGINE_ASSERT(remaining == 0, "No remaining expected.");
			}

			int32 operator[](const int i) const {
				return data[i];
			}
		private:
			std::array<int8, Count> data;
	};

	// TODO: move
	// TODO: name?
	template<int32 Value>
	class ConstantDistribution {
		public:
			int32 operator[](const int i) const {
				return Value;
			}
	};

	// TODO: Different F_n values as template param? (nth distance)
	// TODO: Split? Inline?
	// TODO: Vectorify?
	// TODO: There seems to be some diag artifacts (s = 0.91) in the noise (existed pre RangePermutation)
	// TODO: For large step sizes (>10ish. very noticeable at 100) we can start to notice repetitions in the noise. I suspect this this correlates with the perm table size.
	// TODO: Do those artifacts show up with simplex as well? - They are. But only for whole numbers? If i do 500.02 instead of 500 they are almost imperceptible.
	template<class Distribution>
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

			// TODO: This code makes some assumptions about `Distribution`. We should probably note those or enforce those somewhere.
			// TODO: make the point distribution a arg or calc in construct
			WorleyNoise(int64 seed, const Distribution& dist) : perm{seed}, dist{dist} {
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
			Distribution dist;
	};

	// TODO: move into namesapce ore something
	const static inline auto poisson3 = PoissonDistribution<256, 3, 1, 9>{};
	const static inline auto poisson2 = PoissonDistribution<256, 2, 1, 3>{};

	// TODO: name. WorleyNoiseP3 ?
	class WorleyNoise3 : public WorleyNoise<decltype(poisson3)> {
		public:
			WorleyNoise3(int64 seed) : WorleyNoise{seed, poisson3} {
			}
	};

	// TODO: do this instead. Only change one var.
	// TODO: name.
	template<auto* a = &poisson2>
	class WorleyNoise2 : public WorleyNoise<decltype(*a)> {
		public:
			WorleyNoise2(int64 seed) : WorleyNoise{seed, *a} {
			}
	};

	// TODO: name. WorleyNoiseConst<N> ? or similar
	// TODO: impl
	class WorleyNoise1 : public WorleyNoise<ConstantDistribution<1>> {
		public:
			WorleyNoise1(int64 seed) : WorleyNoise{seed, ConstantDistribution<1>{}} {
			}
	};
}
