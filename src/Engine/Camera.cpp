// GLM
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Camera.hpp>


namespace Engine {
	void Camera::setAsOrtho(unsigned int width, unsigned int height, float scale) {
		this->width = width;
		this->height = height;
		this->scale = scale;

		auto halfWidth = (width / 2.0f) * scale;
		auto halfHeight = (height / 2.0f) * scale;

		projection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);
		glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);
	}

	void Camera::setPosition(const glm::vec2& position) {
		view = glm::translate(glm::mat4{1.0f}, glm::vec3{-position, 0.0f});
	}

	glm::vec3 Camera::getPosition() const {
		return view * glm::vec4(0, 0, 0, 1);
	}

	unsigned int Camera::getWidth() const {
		return width;
	}

	unsigned int Camera::getHeight() const {
		return height;
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
		point = point * 2.0f / glm::vec2(width, height) - glm::vec2(1.0f, 1.0f);

		// In screen space up is negative. In world/ndc space up is positive
		point.y *= -1.0f;

		// Apply camera transforms
		return glm::inverse(view) * glm::inverse(projection) * glm::vec4(point, 0, 1);
	}
}
