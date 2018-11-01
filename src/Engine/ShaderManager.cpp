// Engine
#include <Engine/Engine.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/ShaderManager.hpp>

namespace Engine {
	GLuint ShaderManager::load(const std::string& path) {
		// TODO: Json file or similar instead of this
		const std::string vertPath = path + ".vert";
		const std::string fragPath = path + ".frag";

		// Vertex shader
		auto vertShader = glCreateShader(GL_VERTEX_SHADER);
		{
			const auto source = Engine::Utility::readFile(vertPath);
			const auto cstr = source.c_str();
			glShaderSource(vertShader, 1, &cstr, nullptr);
		}
		glCompileShader(vertShader);

		// Fragment shader
		auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		{
			const auto source = Engine::Utility::readFile(fragPath);
			const auto cstr = source.c_str();
			glShaderSource(fragShader, 1, &cstr, nullptr);
		}
		glCompileShader(fragShader);

		// Shader program
		GLuint shader = glCreateProgram();
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
				ENGINE_ERROR(buffer);
			}
		}

		// Shader cleanup
		glDetachShader(shader, vertShader);
		glDetachShader(shader, fragShader);
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		return shader;
	}

	void ShaderManager::unload(GLuint shader) {
		glDeleteProgram(shader);
	}
}
