#pragma once

// STD
#include <algorithm>
#include <array>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/BaseMember.hpp>
#include <Engine/Noise/noise.hpp>
#include <Engine/Noise/RangePermutation.hpp>
#include <Engine/Noise/Metric.hpp>


// TOOD: Look at "Implementation of Fast and Adaptive Procedural Cellular Noise" http://www.jcgt.org/published/0008/01/02/paper.pdf
namespace Engine::Noise {
	// TODO: move
	// TODO: name?
	template<int32 Value>
	class ConstantDistribution {
		public:
			ENGINE_INLINE constexpr int32 operator()(const int i) const noexcept {
				return Value;
			}
	};

	const static inline auto constant1 = ConstantDistribution<1>{};


	// TODO: edges? https://www.iquilezles.org/www/articles/voronoilines/voronoilines.htm
	// TODO: Vectorify?
	// TODO: 1d, 3d, 4d, versions
	// TODO: There seems to be some diag artifacts (s = 0.91) in the noise (existed pre RangePermutation)
	// TODO: For large step sizes (>10ish. very noticeable at 100) we can start to notice repetitions in the noise. I suspect this this correlates with the perm table size.
	// TODO: Do those artifacts show up with simplex as well? - They are. But only for whole numbers? If i do 500.02 instead of 500 they are almost imperceptible.
	// TODO: Multiple types. Some common: F1Squared, F1, F2, F2 - F1, F2 + F1
	/**
	 * Used to generate Worley noise.
	 * 
	 * The output Worley noise can be influence by a number of parameters:
	 * - The permutation function - maps expected values to pseudo random values.
	 * - The distribution function - controls how many feature points each cell has.
	 * - The metric function - controls what metric is used to determine the smallest value based on surrounding feature points.
	 * 
	 * @see PoissonDistribution
	 * @see ConstantDistribution
	 * @see MetricEuclidean2
	 * @see MetricManhattan
	 * @see MetricChebyshev
	 * @see MetricMinkowski
	 */
	template<class Perm, class Dist, class Metric, std::floating_point Float = float32, std::integral Int = int32>
	class ENGINE_EMPTY_BASE WorleyNoiseGeneric : protected BaseMember<Perm>, BaseMember<Dist>, BaseMember<Metric> {
		public:
			using IVec = glm::vec<2, Int>;
			using FVec = glm::vec<2, Float>;

			class Result {
				public:
					/** The coordinate of the cell the point is in. */
					IVec cell;

					/** The number of the point in the cell. */
					Int n;

					// TODO: Update comment.
					/** The squared distance from the original input point. */
					Float value = std::numeric_limits<Float>::max();
			};

			template<class PermTuple, class DistTuple, class MetricTuple>
			WorleyNoiseGeneric(std::piecewise_construct_t, PermTuple&& permTuple, DistTuple&& distTuple, MetricTuple&& metricTuple)
				// This is fine, even for large objects, due to guaranteed copy elision. See C++ Standard [tuple.apply]
				: BaseMember<Perm>(std::make_from_tuple<BaseMember<Perm>>(std::forward<PermTuple>(permTuple)))
				, BaseMember<Dist>(std::make_from_tuple<BaseMember<Dist>>(std::forward<DistTuple>(distTuple)))
				, BaseMember<Metric>(std::make_from_tuple<BaseMember<Metric>>(std::forward<MetricTuple>(metricTuple))) {
			}

			WorleyNoiseGeneric(Perm perm, Dist dist, Metric metric)
				: BaseMember<Perm>(std::move(perm))
				, BaseMember<Dist>(std::move(dist))
				, BaseMember<Metric>(std::move(metric)) {
			}

			// TODO: doc
			// TODO: name? F1Squared would be more standard
			/**
			 * TODO: finish doc
			 * In this case Result::value is the squared distance to the nearest point.
			 */
			Result valueD2(Float x, Float y) const noexcept {
				Result result;

				evaluate({x, y}, [&](const FVec pos, const FVec point, const IVec cell, Int ci) ENGINE_INLINE {
					const auto m = metric()(pos, point);
					if (m < result.value) {
						result = {cell, ci, m};
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
			Result valueF2F1(Float x, Float y) const noexcept {
				Result result1;
				Result result2;

				evaluate({x, y}, [&](const FVec pos, const FVec point, const IVec cell, Int ci) ENGINE_INLINE {
					const auto m = metric()(pos, point);
					
					if (m < result1.value) {
						result2 = result1;
						result1 = {cell, ci, m};
					} else if (m < result2.value) {
						result2 = {cell, ci, m};
					}
				});

				result1.value = result2.value - result1.value;
				return result1;
			}

		protected:
			[[nodiscard]] ENGINE_INLINE decltype(auto) perm() noexcept { return BaseMember<Perm>::get(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) perm() const noexcept { return BaseMember<Perm>::get(); }

			[[nodiscard]] ENGINE_INLINE decltype(auto) dist() noexcept { return BaseMember<Dist>::get(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) dist() const noexcept { return BaseMember<Dist>::get(); }
			
			[[nodiscard]] ENGINE_INLINE decltype(auto) metric() noexcept { return BaseMember<Metric>::get(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) metric() const noexcept { return BaseMember<Metric>::get(); }

			// TODO: Doc
			template<class PointProcessor>
			void evaluate(const FVec pos, PointProcessor&& pp) const {
				// Figure out which base unit square we are in
				const IVec base = { floorTo<Int>(pos.x), floorTo<Int>(pos.y) };

				// TODO: check boundary cubes. Based on our closest point we can cull rows/cols
				for (IVec offset{-1}; offset.y < 2; ++offset.y) {
					for (offset.x = -1; offset.x < 2; ++offset.x) {
						// Position and points in this cell
						const IVec cell = base + offset;
						const int numPoints = dist()(perm()(cell.x, cell.y));

						// Find the best point in this cell
						for (int i = 0; i < numPoints; ++i) {
							// TODO: this make assumptions about perm output range... either doc or make a getter
							const FVec poff = FVec{ perm()(cell.x, cell.y, +i), perm()(cell.x, cell.y, -i) } * (Float{1} / Float{255});
							const FVec point = FVec{cell} + poff;
							pp(pos, point, cell, i);
						}
					}
				}
			}
	};

	class WorleyNoise : public WorleyNoiseGeneric<RangePermutation<256>, ConstantDistribution<1>, MetricEuclidean2, float32, int32> {
		public:
			WorleyNoise(uint64 seed) : WorleyNoiseGeneric{seed, {}, {}} {
			}
	};
}
