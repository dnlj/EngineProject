#pragma once

// GLM
#include <glm/vec2.hpp>


namespace Engine::UI {
	class Bounds {
		public:
			glm::vec2 min;
			glm::vec2 max;

			ENGINE_INLINE bool contains(const glm::vec2 point) const noexcept {
				return point.x >= min.x
					&& point.y >= min.y
					&& point.x <= max.x
					&& point.y <= max.y;
			}

			ENGINE_INLINE Bounds intersect(const Bounds other) const noexcept {
				return {
					glm::max(min, other.min),
					glm::min(max, other.max),
				};
			}

			ENGINE_INLINE glm::vec2 getSize() const noexcept {
				return max - min;
			}

			ENGINE_INLINE auto getWidth() const noexcept {
				return max.x - min.x;
			}
			
			ENGINE_INLINE auto getHeight() const noexcept {
				return max.y - min.y;
			}

			ENGINE_INLINE glm::vec2 getRoundSize() const noexcept {
				return glm::ceil(max) - glm::floor(min);
			}

			ENGINE_INLINE friend bool operator==(const Bounds& a, const Bounds& b) noexcept {
				return a.min == b.min && a.max == b.max;
			}

			ENGINE_INLINE friend Bounds operator+(const Bounds& a, const glm::vec2 b) noexcept {
				auto res = a;
				res.min += b;
				res.max += b;
				return res;
			}
			
			ENGINE_INLINE friend Bounds operator-(const Bounds& a, const glm::vec2 b) noexcept {
				return a + -b;
			}

	};
}
