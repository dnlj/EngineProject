// GLM
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include<Engine/Camera.hpp>


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
}
