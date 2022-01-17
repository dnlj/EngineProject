#pragma once

// Engine
#include <Engine/ArrayView.hpp>


namespace Engine::Gui {
	class LayoutMetrics {
		public:
			float32 min = 0;
			float32 max = 0;
			float32 val = 0;
			float32 weight = 0;
			float32 pos = 0;

		public:
			static void distribute(
				ArrayView<LayoutMetrics> data,
				const float32 totalSize,
				const float32 totalWeight,
				const float32 gap) {

				const auto sz = data.size();
				auto remSize = totalSize - gap * (sz - 1);
				auto remWeight = totalWeight;

				while (remSize >= 1 && remWeight > 0) {
					auto runWeight = remWeight;
					for (auto& d : data) {
						if (!d.weight) { continue; }
						if (d.val == d.max) { continue; }

						const auto w = d.weight / runWeight;
						const auto v = w * remSize;
						const auto o = d.val;
						d.val = std::clamp(d.val + v, d.min, d.max);

						remSize -= d.val - o;
						runWeight -= d.weight;

						// If we dont do this check we will never exit if all items are max size and we have size left to distribute
						if (d.val == d.max) { remWeight -= d.weight; }
					}
				}
			}
	};
}
