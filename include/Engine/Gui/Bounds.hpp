#pragma once

// GLM
#include <glm/vec2.hpp>


namespace Engine::Gui {
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

			ENGINE_INLINE glm::vec2 getSize() const noexcept {
				return max - min;
			}

			ENGINE_INLINE glm::vec2 getRoundSize() const noexcept {
				return glm::ceil(max) - glm::floor(min);
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
