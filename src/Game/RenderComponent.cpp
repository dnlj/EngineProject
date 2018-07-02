// STD
#include <iostream>

// Engine
#include <Engine/Debug/Debug.hpp>
#include <Engine/Utility/Utility.hpp>

// Game
#include <Game/RenderComponent.hpp>

namespace Game {
	RenderComponent::~RenderComponent() {
		// TODO: shouldnt do this in deconstructor
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteProgram(shader);
	}

	void RenderComponent::setup(Engine::TextureManager& textureManager) {
		constexpr GLfloat data[] = {
		//   x     y       u     v
			-0.5, +0.5,   +0.0, +1.0,
			-0.5, -0.5,   +0.0, +0.0,
			+0.5, -0.5,   +1.0, +0.0,

			+0.5, -0.5,   +1.0, +0.0,
			+0.5, +0.5,   +1.0, +1.0,
			-0.5, +0.5,   +0.0, +1.0,
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

		// Fragment shader
		auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		{
			const auto source = Engine::Utility::readFile("shaders/fragment.glsl");
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
}
