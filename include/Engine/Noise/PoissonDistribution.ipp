#pragma once

// STD
#include <cstdint>
#include <cmath>

// Engine
#include <Engine/Noise/PoissonDistribution.hpp>


namespace Engine::Noise {
	template<int Count, int Mean, int Min, int Max>
	PoissonDistribution<Count, Mean, Min, Max>::PoissonDistribution() {
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
			const auto dataP = data.data() + dataOffset;
			std::fill(dataP, dataP + amount, cur);
			dataOffset += amount;
		}

		ENGINE_DEBUG_ASSERT(remaining == 0, "No remaining expected.");
	}
	template<int Count, int Mean, int Min, int Max>
	int32 PoissonDistribution<Count, Mean, Min, Max>::operator[](const int i) const {
		return data[i];
	}
}
