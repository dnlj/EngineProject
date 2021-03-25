#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Noise/RangePermutation.hpp>


namespace Engine::Noise {
	class SimplexNoise {
		public:
			using Int = int32;
			using Float = float32;

		private:
			const RangePermutation<256> perm;

			// We exclude the extremes to get more visually appealing results
			constexpr static Float grads[] = {
				//0.198912367f,
				0.414213562f,
				0.668178638f,
				1.000000000f,
				1.496605763f,
				//2.414213562f,
				//5.027339492f,
				//10.00000000f, // Really would be +inf
				//
				//-0.198912367f,
				-0.414213562f,
				-0.668178638f,
				-1.000000000f,
				-1.496605763f,
				//-2.414213562f,
				//-5.027339492f,
				//-10.00000000f, // Really would be -inf
			};


		public:
			SimplexNoise(uint64 seed) : perm{seed} {}

			// TODO: why does this still look so bad compared to using a 2d simplex with constant y?
			[[nodiscard]]
			Float value1D(const Float x) const noexcept {
				const auto prev = floorTo<Int>(x);
				const auto next = prev + 1;

				// TODO: Consider scewing distances here. Might contribute to the strange looks. Some simplex impls do that.
				// Product of gradient and distance
				const auto dot1 = grad(prev) * (prev - x);
				const auto dot2 = grad(next) * (next - x);

				auto p = x - prev;
				//p = (3 - 2 * p) * p * p; // 3rd order smoothstep
				p = ((6*p - 15)*p + 10)*p*p*p; // 5th order smoothstep

				// TODO: Shouldnt this be 1/xyz ? why two
				constexpr auto scale = 2.0f / *std::max_element(std::cbegin(grads), std::cend(grads));
				//constexpr auto scale = 2.0f / 7.0f; // if using generated grad instead of lookup
				return (dot1 + p * (dot2 - dot1)) * scale;
			}

			// TODO: 2D - look into opensimplex2
			// TODO: 3D - look into opensimplex2
			// TODO: 4D - look into opensimplex2

		private:
			ENGINE_INLINE Float grad(const Int i) const noexcept {
				// Treat the perm value as a 3 bit number + sign
				//const auto h = perm.value(i);
				//const auto v = 1 + h & 0b0111;
				//return static_cast<Float>(!(h & 0b1000) ? v : -v);

				//return grads[perm.value(i) % std::size(grads)];
				return grads[perm.value(i) & 0b0111];
			}
	};
}
