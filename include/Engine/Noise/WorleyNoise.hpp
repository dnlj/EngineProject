#pragma once

// STD
#include <algorithm>
#include <array>

// GLM
#include <glm/gtx/norm.hpp>

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

	// TODO: make perm and dist functors so they can be either hash functions or 
	// TODO: use EBO for perm, dist, and metric functions - __declspec(empty_bases) - no_unique_address doesnt work on MSVC yet. See: https://devblogs.microsoft.com/cppblog/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/
	// TODO: Split? Inline?
	// TODO: Vectorify?
	// TODO: There seems to be some diag artifacts (s = 0.91) in the noise (existed pre RangePermutation)
	// TODO: For large step sizes (>10ish. very noticeable at 100) we can start to notice repetitions in the noise. I suspect this this correlates with the perm table size.
	// TODO: Do those artifacts show up with simplex as well? - They are. But only for whole numbers? If i do 500.02 instead of 500 they are almost imperceptible.
	// TODO: Version/setting for distance type (Euclidean, Manhattan, Chebyshev, Minkowski)
	// TODO: Multiple types. Some common: F1Squared, F1, F2, F2 - F1, F2 + F1
	template<bool PermRef, class Perm, bool DistRef, class Dist>
	class WorleyNoiseGeneric {
		protected:
			using PermType = std::decay_t<Perm>;
			using PermStore = std::conditional_t<PermRef, const PermType&, PermType>;
			const PermStore perm;

			using DistType = std::decay_t<Dist>;
			using DistStore = std::conditional_t<DistRef, const DistType&, DistType>;
			const DistStore dist;

		public:
			// For easy changing later
			// TODO: template params?
			using Float = float32;
			using Int = int32;
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

			// TODO: This code makes some assumptions about `Distribution`. We should probably note those or enforce those somewhere.
			// TODO: make the point distribution a arg or calc in construct
			WorleyNoiseGeneric(const PermStore& perm, const DistStore& dist) : perm{perm}, dist{dist} {
			}

			//void setSeed(int64 seed) {
			//	perm = seed;
			//}

			// TODO: doc
			// TODO: name? F1Squared would be more standard
			/**
			 * TODO: finish doc
			 * In this case Result::value is the squared distance to the nearest point.
			 */
			Result valueD2(Float x, Float y) const noexcept {
				Result result;

				evaluate({x, y}, [&](const FVec pos, const FVec point, const IVec cell, Int ci) ENGINE_INLINE {
					const Float d2 = glm::length2(point - pos);
					if (d2 < result.value) {
						result = {cell, ci, d2};
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
					const Float d2 = glm::length2(point - pos);
					
					if (d2 < result1.value) {
						result2 = result1;
						result1 = {cell, ci, d2};
					} else if (d2 < result2.value) {
						result2 = {cell, ci, d2};
					}
				});

				result1.value = result2.value - result1.value;
				return result1;
			}

		protected:
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
						const int numPoints = dist[perm.value(cell.x, cell.y)];

						// Find the best point in this cell
						for (int i = 0; i < numPoints; ++i) {
							const FVec poff = FVec{ perm.value(cell.x, cell.y, +i), perm.value(cell.x, cell.y, -i) } * (Float{1} / Float{255});
							const FVec point = FVec{cell} + poff;
							pp(pos, point, cell, i);
						}
					}
				}
			}
	};

	inline static const RangePermutation<256> TODO_rm_perm = 1234567890;

	// TODO: Doc
	template<auto* Dist>
	class WorleyNoiseFrom : public WorleyNoiseGeneric<false, decltype(TODO_rm_perm), true, decltype(*Dist)> {
		public:
			WorleyNoiseFrom(int64 seed) : WorleyNoiseGeneric<false, decltype(TODO_rm_perm), true, decltype(*Dist)>{seed, *Dist} {
			}
	};

	// TODO: doc
	class WorleyNoise : public WorleyNoiseFrom<&poisson3> {
	};

}
