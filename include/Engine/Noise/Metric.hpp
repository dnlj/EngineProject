#pragma once

// GLM
#include <glm/gtx/norm.hpp>
#include <glm/common.hpp>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Noise {
	/**
	 * Computes the squared euclidean distance between two points.
	 */
	class MetricEuclidean2 {
		public:
			template<class Vec>
			[[nodiscard]] ENGINE_INLINE decltype(auto) operator()(const Vec& a, const Vec& b) const noexcept {
				return glm::length2(b - a);
			}
	};
	
	/**
	 * Computes the Manhattan distance between two points.
	 */
	class MetricManhattan {
		public:
			template<class Vec>
			[[nodiscard]] ENGINE_INLINE decltype(auto) operator()(const Vec& a, const Vec& b) const noexcept {
				static_assert(Vec::length() == 2, "TODO: impl for more vector sizes");
				// Some reason if we use glm funcs they aren't inlined even with GLM_FORCE_INLINE and resulted in code that is twice as slow (in release mode)
				return abs(b.x-a.x) + abs(b.y-a.y);
			}
	};
	
	/**
	 * Computes the Chebyshev distance between two points.
	 */
	class MetricChebyshev {
		public:
			template<class Vec>
			[[nodiscard]] ENGINE_INLINE decltype(auto) operator()(const Vec& a, const Vec& b) const noexcept {
				static_assert(Vec::length() == 2, "TODO: impl for more vector sizes");
				// Some reason GLM functions are slower even in release mode. Even more so in debug.
				return std::max(abs(b.x-a.x), abs(b.y-a.y));
			}
	};
	
	/**
	 * Computes the Minkowski distance of order @p N between two points.
	 */
	template<int N>
	class MetricMinkowski {
		public:
			template<class Vec>
			[[nodiscard]] ENGINE_INLINE decltype(auto) operator()(const Vec& a, const Vec& b) const noexcept {
				// TODO: impl
			}
	};
}
