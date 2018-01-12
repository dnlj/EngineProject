#pragma once

// STD
#include <array>
#include <vector>

// GLM
#include <glm/glm.hpp>

// Box2D
#include <Box2D/Box2D.h>

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

			void reset();
			void draw();

			virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
			virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
			virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override;
			virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override;
			virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
			void DrawSegmentInside(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
			virtual void DrawTransform(const b2Transform& xf) override;
			virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override;

		private:
			// TODO: Make these constructor arguments instead of constants
			static constexpr float LINE_SIZE = 0.008f;
			static constexpr float AXIS_SIZE = 0.1f;
			static constexpr float FILL_COLOR_MULT = 0.5f;
			static constexpr float MAX_CIRCLE_ERROR = 0.005f;
			static constexpr unsigned int MIN_CIRCLE_VERTICES = 8;

			std::array<Vertex, 512> vertexData;
			size_t vertexCount = 0;
			GLuint vao;
			GLuint vbo;
			GLuint shader;

			void addVertex(Vertex vertex);
			std::vector<b2Vec2> getCircleVertices(const b2Vec2& center, float32 radius) const;
	};
}