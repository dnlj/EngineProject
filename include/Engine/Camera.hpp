#pragma once

// GLM
#include <glm/glm.hpp>

namespace Engine {
	class Camera {
		public:
			glm::mat4 projection;
			glm::mat4 view;
	};
}
