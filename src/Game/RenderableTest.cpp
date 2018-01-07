// STD
#include <iostream>

// Engine
#include <Engine/Utility/Utility.hpp>
#include <Engine/ECS/ECS.hpp>

// Game
#include <Game/RenderableTest.hpp>

namespace Game {
	RenderableTest::~RenderableTest() {
		// TODO: shouldnt do this in deconstructor
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteProgram(shader);
	}

	void RenderableTest::setup(Engine::TextureManager& textureManager, b2World& world) {
		constexpr GLfloat data[] = {
			+0.0f, +0.5f, +0.5, +0.0f,
			-0.5f, -0.5f, +0.0, +1.0f,
			+0.5f, -0.5f, +1.0, +1.0f,
		};

		texture = textureManager.getTexture("../assets/test.png");

		// VAO
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// VBO
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);

		// Vertex attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<const void*>(2 * sizeof(GLfloat)));

		// Vertex shader
		auto vertShader = glCreateShader(GL_VERTEX_SHADER);
		{
			const auto source = Engine::Utility::readFile("shaders/vertex.glsl");
			const auto cstr = source.c_str();
			glShaderSource(vertShader, 1, &cstr, nullptr);
		}
		glCompileShader(vertShader);

		{
			GLint status;
			glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);

			if (!status) {
				char buffer[512];
				glGetShaderInfoLog(vertShader, 512, NULL, buffer);
				std::cout << buffer << std::endl;
			}
		}

		// Fragment shader
		auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		{
			const auto source = Engine::Utility::readFile("shaders/fragment.glsl");
			const auto cstr = source.c_str();
			glShaderSource(fragShader, 1, &cstr, nullptr);
		}
		glCompileShader(fragShader);

		{
			GLint status;
			glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);

			if (!status) {
				char buffer[512];
				glGetShaderInfoLog(fragShader, 512, NULL, buffer);
				std::cout << buffer << std::endl;
			}
		}

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

		// Box2D
		{
			b2BodyDef bodyDef;
			bodyDef.type = b2_dynamicBody;

			body = world.CreateBody(&bodyDef);

			b2CircleShape shape;
			shape.m_radius = 0.25f;

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &shape;
			fixtureDef.density = 1.0f;

			body->CreateFixture(&fixtureDef);
		}
	}

	ENGINE_REGISTER_COMPONENT(RenderableTest);
}
