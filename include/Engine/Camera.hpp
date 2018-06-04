#pragma once

// GLM
#include <glm/glm.hpp>

namespace Engine {
	class Camera {
		public:
			glm::mat4 projection{1.0f};
			glm::mat4 view{1.0f};
	};
}
