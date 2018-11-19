// STD
#include <iostream>
#include <cmath>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

// Engine
#include <Engine/Debug/Debug.hpp>
#include <Engine/Debug/DebugDrawBox2D.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/Engine.hpp>


namespace Engine::Debug {
	DebugDrawBox2D::DebugDrawBox2D() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData.data(), GL_DYNAMIC_DRAW);

		// Vertex attributes
		constexpr auto stride = sizeof(b2Vec2) + sizeof(b2Color);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(sizeof(b2Vec2)));

		// Vertex shader
		auto vertShader = glCreateShader(GL_VERTEX_SHADER);
		{
			const auto source = Engine::Utility::readFile("shaders/box2d_debug.vert");
			const auto cstr = source.c_str();
			glShaderSource(vertShader, 1, &cstr, nullptr);
		}
		glCompileShader(vertShader);

		// Fragment shader
		auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		{
			const auto source = Engine::Utility::readFile("shaders/box2d_debug.frag");
			const auto cstr = source.c_str();
			glShaderSource(fragShader, 1, &cstr, nullptr);
		}
		glCompileShader(fragShader);

		// Shader program
		shader = glCreateProgram();
		glAttachShader(shader, vertShader);
		glAttachShader(shader, fragShader);
		glLinkProgram(shader);

		{
			GLint status;
			glGetProgramiv(shader, GL_LINK_STATUS, &status);

			if (!status) {
				char buffer[512];
				glGetProgramInfoLog(shader, 512, NULL, buffer);
				std::cout << buffer << std::endl;
			}
		}

		// Shader cleanup
		glDetachShader(shader, vertShader);
		glDetachShader(shader, fragShader);
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);
	}

	DebugDrawBox2D::~DebugDrawBox2D() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteProgram(shader);
	}

	void DebugDrawBox2D::setup(Camera& camera) {
		this->camera = &camera;
	}

	void DebugDrawBox2D::reset() {
		vertexCount = 0;
		vertexWarning = true;
	}

	void DebugDrawBox2D::draw() {
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(Vertex), vertexData.data());

		glUseProgram(shader);

		glm::mat4 pv = camera->getProjection() * camera->getView();
		glUniformMatrix4fv(2, 1, GL_FALSE, &pv[0][0]);

		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));

		{ // Update screen bounds
			const auto tl = camera->screenToWorld({0, 0});
			const auto br = camera->screenToWorld(camera->getScreenSize());

			screenBoundsMin = {tl.x, br.y};
			screenBoundsMax = {br.x, tl.y};
		}
	}

	void DebugDrawBox2D::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
		for (int32 i = 0; i < vertexCount - 1; ++i) {
			DrawSegmentInside(vertices[i], vertices[i + 1], color);
		}

		DrawSegmentInside(vertices[vertexCount - 1], vertices[0], color);
	}

	void DebugDrawBox2D::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
		b2Color fillColor{FILL_COLOR_MULT * color.r, FILL_COLOR_MULT * color.g, FILL_COLOR_MULT * color.b, FILL_COLOR_MULT * color.a};

		for (int32 i = 1; i < vertexCount - 1; ++i) {
			addVertex(Vertex{vertices[0], fillColor});
			addVertex(Vertex{vertices[i], fillColor});
			addVertex(Vertex{vertices[i + 1], fillColor});
		}

		DrawPolygon(vertices, vertexCount, color);
	}

	void DebugDrawBox2D::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {
		if (shouldCullCircle(center, radius)) { return; };

		const auto vertices = getCircleVertices(center, radius);
		DrawPolygon(vertices.data(), static_cast<int>(vertices.size()), color);
	}

	void DebugDrawBox2D::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {
		if (shouldCullCircle(center, radius)) { return; };

		const auto vertices = getCircleVertices(center, radius);
		DrawSolidPolygon(vertices.data(), static_cast<int>(vertices.size()), color);
		DrawSegment(center, center + radius * axis, color);
	}

	void DebugDrawBox2D::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
		if (shouldCullLineSegment(p1, p2)) { return; }

		// Get a scaled normal vector
		b2Vec2 normal{p2.y - p1.y, -p2.x + p1.x};
		normal.Normalize();
		normal *= LINE_SIZE / 2.0f;

		// Compute points
		auto v1 = p1 - normal;
		auto v2 = p1 + normal;
		auto v3 = p2 + normal;
		auto v4 = p2 - normal;

		// Add the data
		addVertex(Vertex{v1, color});
		addVertex(Vertex{v2, color});
		addVertex(Vertex{v3, color});

		addVertex(Vertex{v3, color});
		addVertex(Vertex{v4, color});
		addVertex(Vertex{v1, color});
	}

	void DebugDrawBox2D::DrawSegmentInside(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
		if (shouldCullLineSegment(p1, p2)) { return; }

		// Get a scaled normal vector
		b2Vec2 normal{p2.y - p1.y, -p2.x + p1.x};
		normal.Normalize();
		normal *= LINE_SIZE;

		// Compute points
		auto v1 = p1 - normal;
		auto v2 = p1;
		auto v3 = p2;
		auto v4 = p2 - normal;

		// Add the data
		addVertex(Vertex{v1, color});
		addVertex(Vertex{v2, color});
		addVertex(Vertex{v3, color});

		addVertex(Vertex{v3, color});
		addVertex(Vertex{v4, color});
		addVertex(Vertex{v1, color});
	}

	void DebugDrawBox2D::DrawTransform(const b2Transform& xf) {
		DrawSegment(xf.p, xf.p + AXIS_SIZE * xf.q.GetXAxis(), b2Color{1.0f, 0.0f, 0.0f});
		DrawSegment(xf.p, xf.p + AXIS_SIZE * xf.q.GetYAxis(), b2Color{0.0f, 1.0f, 0.0f});
	}

	void DebugDrawBox2D::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) {
		ENGINE_WARN("TODO: DrawPoint is not yet implemented.");
	}

	bool DebugDrawBox2D::shouldCullLineSegment(const b2Vec2& p1, const b2Vec2& p2) {
		const glm::vec2 aabbMin = {std::min(p1.x, p2.x), std::min(p1.y, p2.y)};
		const glm::vec2 aabbMax = {std::max(p1.x, p2.x), std::max(p1.y, p2.y)};

		return glm::all(glm::lessThan(aabbMax, screenBoundsMin))
			|| glm::all(glm::greaterThan(aabbMin, screenBoundsMax));
	}

	bool DebugDrawBox2D::shouldCullCircle(const b2Vec2& center, float32 radius) {
		const glm::vec2 min = {center.x + radius, center.y + radius};
		const glm::vec2 max = {center.x - radius, center.y - radius};

		return glm::any(glm::lessThan(min, screenBoundsMin))
			|| glm::any(glm::greaterThan(max, screenBoundsMax));
	}

	void DebugDrawBox2D::addVertex(Vertex vertex) {
		if (vertexCount == vertexData.size()) {
			if (vertexWarning) {
				ENGINE_WARN("To many debug vertices. Increase MAX_VERTICES");
				vertexWarning = false;
			}
		} else {
			// SRGB framebuffer correction
			vertex.color.r = powf(vertex.color.r, 2.2f);
			vertex.color.g = powf(vertex.color.g, 2.2f);
			vertex.color.b = powf(vertex.color.b, 2.2f);
			vertex.color.a = powf(vertex.color.a, 2.2f);

			vertexData[vertexCount] = vertex;
			++vertexCount;
		}
	}

	std::vector<b2Vec2> DebugDrawBox2D::getCircleVertices(const b2Vec2& center, float32 radius) const {
		// Calculate the number of vertices needed.
		// This formula is derived from the fact that:
		//     d = r - rcos(t/2)
		// Where:
		//     n = the number of vertices needed (solve for this)
		//     r = the radius
		//     t = theta = 2pi/n = the angle between two vertices
		//     d = the maximum distance from the line between two vertices and the true circle
		// See notes for more detail
		const unsigned int vertCount = std::max(
			MIN_CIRCLE_VERTICES,
			static_cast<unsigned int>(std::ceil(
				glm::pi<float>() / std::acos(1.0f - (MAX_CIRCLE_ERROR / radius))
			))
		);
		
		// The angle by which to increment each vertex;
		const float angleInc = glm::two_pi<float>() / vertCount;
		std::vector<b2Vec2> vertices{vertCount};

		// Generate our vertices
		for (unsigned int i = 0; i < vertCount; ++i) {
			vertices[i].x = cos(i * angleInc) * radius;
			vertices[i].y = sin(i * angleInc) * radius;
			vertices[i] += center;
		}

		return vertices;
	}
}
