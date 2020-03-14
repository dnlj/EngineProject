#pragma once

// GLM
#include <glm/glm.hpp>


namespace Engine {
	class Camera {
		public:
			struct ScreenBounds {
				glm::vec2 min;
				glm::vec2 max;
			};

		public:
			/**
			 * Sets this camera to use an orthographic projection.
			 */
			void setAsOrtho(int width, int height, float scale);

			/**
			 * @return The width of the screen in pixels.
			 */
			int getWidth() const;

			/**
			 * @return The height of the screen in pixels.
			 */
			int getHeight() const;

			/**
			 * @return The scale of world units to pixels.
			 */
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

			/**
			 * @return The screen size in pixels.
			 */
			const glm::ivec2& getScreenSize() const;

			/**
			 * @return The bounds of the screen in world coordinates.
			 */
			const Camera::ScreenBounds& getWorldScreenBounds() const;

		private:
			void setProjection(glm::mat4 m);
			void setView(glm::mat4 m);
			void updateScreenBounds();

			glm::mat4 view{1.0f};
			glm::mat4 viewInv;

			glm::mat4 proj{1.0f};
			glm::mat4 projInv;

			glm::vec3 position{0.0f};
			glm::ivec2 size = {0, 0};
			float scale = 1.0f;

			ScreenBounds screenBounds;
	};
}
