#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Noise {
	// TODO: Doc
	template<int Count, int Mean, int Min, int Max>
	class PoissonDistribution {
		static_assert(Min < Mean);
		static_assert(Max > Mean);
		public:
			PoissonDistribution() {
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
					const auto dataP = data + dataOffset;
					std::fill(dataP, dataP + amount, cur);
					dataOffset += amount;
				}

				ENGINE_DEBUG_ASSERT(remaining == 0, "No remaining expected.");
			}

			template<class... Args>
			ENGINE_INLINE decltype(auto) operator()(Args&&... args) const { return operator[](std::forward<Args>(args)...); }

			ENGINE_INLINE int32 operator[](const int i) const {
				return data[i];
			}

		private:
			int8 data[Count];
	};

	const static inline auto poisson2 = PoissonDistribution<256, 2, 1, 10>{};
	const static inline auto poisson3 = PoissonDistribution<256, 3, 1, 10>{};
	const static inline auto poisson4 = PoissonDistribution<256, 4, 1, 10>{};
}

