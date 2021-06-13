// Engine
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Utility/Utility.hpp>


namespace Engine::Graphics {
	Shader::Shader(const std::string& path) {
		load(path);
	}

	Shader::Shader(Shader&& other) noexcept {
		*this = std::move(other);
	}

	Shader& Shader::operator=(Shader&& other) noexcept  {
		using std::swap;
		swap(program, other.program);
		return *this;
	}

	Shader::~Shader() {
		unload();
	}

	void Shader::load(const std::string& path) {
		unload();

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
		program = glCreateProgram();
		glAttachShader(program, vertShader);
		glAttachShader(program, fragShader);
		glLinkProgram(program);

		{
			GLint param;
			glGetProgramiv(program, GL_LINK_STATUS, &param);

			if (!param) {
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &param);
				std::string buffer;
				buffer.resize(param);
				glGetProgramInfoLog(program, param, nullptr, buffer.data());
				ENGINE_ERROR("Unable to link OpenGL program: ", buffer);
			}
		}

		// Shader cleanup
		glDetachShader(program, vertShader);
		glDetachShader(program, fragShader);
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);
	}

	void Shader::unload() {
		glDeleteProgram(program);
	}
}
