#pragma once

// GLM
#include <glm/vec2.hpp>


namespace Engine::Gui {
	class Bounds {
		public:
			glm::vec2 topLeft;
			glm::vec2 bottomRight;

			ENGINE_INLINE bool contains(const glm::vec2 point) const noexcept {
				return point.x >= topLeft.x
					&& point.y >= topLeft.y
					&& point.x <= bottomRight.x
					&& point.y <= bottomRight.y;
			}

			ENGINE_INLINE glm::vec2 getSize() const noexcept {
				return bottomRight - topLeft;
			}

			ENGINE_INLINE glm::vec2 getRoundSize() const noexcept {
				return glm::ceil(bottomRight) - glm::floor(topLeft);
			}

			ENGINE_INLINE friend Bounds operator+(const Bounds& a, const glm::vec2 b) noexcept {
				auto res = a;
				res.topLeft += b;
				res.bottomRight += b;
				return res;
			}

	};
}
