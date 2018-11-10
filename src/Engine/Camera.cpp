// GLM
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Camera.hpp>


namespace Engine {
	void Camera::setAsOrtho(int width, int height, float scale) {
		size = {width, height};
		this->scale = scale;

		const auto half = glm::vec2{size} / 2.0f * scale;
		projection = glm::ortho(-half.x, half.x, -half.y, half.y);
	}

	void Camera::setPosition(const glm::vec2 newPosition) {
		position = glm::vec3(newPosition, 0.0f);
		view = glm::translate(glm::mat4{1.0f}, -position);
	}

	glm::vec3 Camera::getPosition() const {
		return position;
	}

	int Camera::getWidth() const {
		return size.x;
	}

	int Camera::getHeight() const {
		return size.y;
	}

	const glm::ivec2& Camera::getScreenSize() const {
		return size;
	}

	float Camera::getScale() const {
		return scale;
	}

	const glm::mat4& Camera::getProjection() const {
		return projection;
	}

	const glm::mat4& Camera::getView() const {
		return view;
	}

	glm::vec2 Camera::screenToWorld(glm::vec2 point) const {
		// Convert from screen space to normalized device coordinates. That is: from [0, width] to [-1, 1]
		point = point * 2.0f / glm::vec2{size} - 1.0f;

		// In screen space up is negative. In world/ndc space up is positive
		point.y *= -1.0f;

		// Apply camera transforms
		return glm::inverse(view) * glm::inverse(projection) * glm::vec4(point, 0, 1);
	}
}
