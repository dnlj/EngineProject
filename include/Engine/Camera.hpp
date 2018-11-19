#pragma once

// GLM
#include <glm/glm.hpp>


// TODO: Doc
namespace Engine {
	class Camera {
		public:
			struct ScreenBounds {
				glm::vec2 min;
				glm::vec2 max;
			};

		public:
			void setAsOrtho(int width, int height, float scale);
			int getWidth() const;
			int getHeight() const;
			float getScale() const;

			/**
			 * @param[in] newPosition The position vector.
			 */
			void setPosition(const glm::vec2 newPosition);

			/**
			 * @return The position vector.
			 */
			const glm::vec3& getPosition() const;

			/**
			 * @return The projection matrix.
			 */
			const glm::mat4& getProjection() const;

			/**
			 * @return The view matrix.
			 */
			const glm::mat4& getView() const;

			/**
			 * Converts a point from screen space to world space.
			 * @return The world space position of the point.
			 */
			glm::vec2 screenToWorld(glm::vec2 point) const;

			// TODO: Doc
			const glm::ivec2& getScreenSize() const;

			// TODO: Doc
			const Camera::ScreenBounds& getWorldScreenBounds() const;

		private:
			void setProjection(glm::mat4 m);
			void setView(glm::mat4 m);
			void updateScreenBounds();

			glm::mat4 view{1.0f};
			glm::mat4 viewInv;

			glm::mat4 proj{1.0f};
			glm::mat4 projInv;

			glm::vec3 position;
			glm::ivec2 size = {0, 0};
			float scale = 1.0f;

			ScreenBounds screenBounds;
	};
}
