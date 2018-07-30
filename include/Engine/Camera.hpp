#pragma once

// GLM
#include <glm/glm.hpp>


// TODO: Doc
namespace Engine {
	class Camera {
		public:
			glm::mat4 projection{1.0f};
			glm::mat4 view{1.0f};

			void setAsOrtho(unsigned int width, unsigned int height, float scale);
			unsigned int getWidth() const;
			unsigned int getHeight() const;
			// TODO: const glm::mat4& getProjection() const;
			// TODO: const glm::mat4& getView() const;

		private:
			unsigned int width = 0;
			unsigned int height = 0;
			float scale = 1.0f;
	};
}
