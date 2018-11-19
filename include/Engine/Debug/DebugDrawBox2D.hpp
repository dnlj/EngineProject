#pragma once

// STD
#include <array>
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/glm.hpp>

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Camera.hpp>

namespace Engine::Debug {
	class DebugDrawBox2D : public b2Draw {
		private:
			struct Vertex {
				public:
					b2Vec2 pos;
					b2Color color;
			};

		public:
			DebugDrawBox2D();
			~DebugDrawBox2D();

			// TODO: Doc
			void setup(Camera& camera);

			/**
			 * @brief Resets the vertices to be drawn next frame.
			 */
			void reset();

			/**
			 * @brief Draws the vertices currently in the vertex data.
			 */
			void draw();

			/** @brief see Box2D documentation. */
			virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

			/** @brief see Box2D documentation. */
			virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

			/** @brief see Box2D documentation. */
			virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override;

			/** @brief see Box2D documentation. */
			virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override;

			/** @brief see Box2D documentation. */
			virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

			/** @brief see Box2D documentation. */
			virtual void DrawTransform(const b2Transform& xf) override;

			/** @brief see Box2D documentation. */
			virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override;

		private:
			// TODO: Make these constructor arguments instead of constants
			/** The thickness of lines and borders */
			static constexpr float LINE_SIZE = 0.008f;

			/** The size of the axis lines when drawing a transform */
			static constexpr float AXIS_SIZE = 0.1f;

			/** The multiplier to use for fill colors */
			static constexpr float FILL_COLOR_MULT = 0.5f;

			/** The maximum distance from the line between two vertices and the true circle */
			static constexpr float MAX_CIRCLE_ERROR = 0.005f;

			/** The minimum number of vertices to have in a circle */
			static constexpr unsigned int MIN_CIRCLE_VERTICES = 8;

			/** The vertex data */
			std::array<Vertex, 1ull << 14> vertexData;

			/** The number of elements in the vertex data */
			size_t vertexCount = 0;

			/** The OpenGL vertex array */
			GLuint vao;

			/** The OpenGL vertex buffer */
			GLuint vbo;

			/** The OpenGL shader program */
			GLuint shader;

			/** Used to prevent vertex warning */
			bool vertexWarning = true;

			// TODO: Doc
			Camera* camera = nullptr;

			// TODO: Doc
			glm::vec2 screenBoundsMin;
			glm::vec2 screenBoundsMax;

			// TODO: Doc
			bool shouldCullLineSegment(const b2Vec2& p1, const b2Vec2& p2);
			bool shouldCullCircle(const b2Vec2& center, float32 radius);


			/**
			 * @brief Adds a vertex the vertex data to draw.
			 * @param[in] vertex The vertex to add.
			 */
			void addVertex(Vertex vertex);

			/**
			 * @brief Gets the vertices for a circle from a center and a radius.
			 * @param[in] center The center of the circle.
			 * @param[in] radius The radius of the circle.
			 * @return A vector containing the vertices.
			 */
			std::vector<b2Vec2> getCircleVertices(const b2Vec2& center, float32 radius) const;

			/**
			 * @brief Draws the inside (CCW) of a segment between two points.
			 * @param[in] p1 The first point.
			 * @param[in] p2 The second point.
			 * @param[in] color The color of the segment.
			 */
			void DrawSegmentInside(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	};
}
