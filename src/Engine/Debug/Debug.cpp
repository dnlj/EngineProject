// Engine
#include <Engine/Debug/Debug.hpp>
#include <Engine/Engine.hpp>

namespace Engine::Debug {
	void checkOpenGLErrors() {
		for (auto error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
			ENGINE_DEBUG(openGLErrorToString(error));
		}
	}

	void checkOpenGLShaderCompilation(GLuint shader) {
		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			GLint length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

			std::string info(length, '\0');
			glGetShaderInfoLog(shader, length, nullptr, info.data());
			ENGINE_WARN(info);
		}
	}

	std::string openGLErrorToString(const GLint error) {
		switch (error) {
			case GL_NO_ERROR:
				return "GL_NO_ERROR";
			case GL_INVALID_ENUM:
				return "GL_INVALID_ENUM";
			case GL_INVALID_VALUE:
				return "GL_INVALID_VALUE";
			case GL_INVALID_OPERATION:
				return "GL_INVALID_OPERATION";
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				return "GL_INVALID_FRAMEBUFFER_OPERATION";
			case GL_OUT_OF_MEMORY:
				return "GL_OUT_OF_MEMORY";
			case GL_STACK_UNDERFLOW:
				return "GL_STACK_UNDERFLOW";
			case GL_STACK_OVERFLOW:
				return "GL_STACK_OVERFLOW";
			default:
				return "UNKNOWN(" + std::to_string(error) + ")";
		}
	}

	void openGLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
		// TODO: Use ENGINE_WARN?
		// TODO: move into functions

		const auto sourceToString = [](GLenum source) {
			switch (source) {
				case GL_DEBUG_SOURCE_API: return "GL_DEBUG_SOURCE_API";
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
				case GL_DEBUG_SOURCE_SHADER_COMPILER: return "GL_DEBUG_SOURCE_SHADER_COMPILER";
				case GL_DEBUG_SOURCE_THIRD_PARTY: return "GL_DEBUG_SOURCE_THIRD_PARTY";
				case GL_DEBUG_SOURCE_APPLICATION: return "GL_DEBUG_SOURCE_APPLICATION";
				case GL_DEBUG_SOURCE_OTHER: return "GL_DEBUG_SOURCE_OTHE";
				default: return "Unknown GL_DEBUG_SOURCE_* enum";
			}
		};

		const auto typeToString = [](GLenum type){
			switch (type) {
				case GL_DEBUG_TYPE_ERROR: return "GL_DEBUG_TYPE_ERROR";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
				case GL_DEBUG_TYPE_PORTABILITY: return "GL_DEBUG_TYPE_PORTABILITY";
				case GL_DEBUG_TYPE_PERFORMANCE: return "GL_DEBUG_TYPE_PERFORMANCE";
				case GL_DEBUG_TYPE_MARKER: return "GL_DEBUG_TYPE_MARKER";
				case GL_DEBUG_TYPE_PUSH_GROUP: return "GL_DEBUG_TYPE_PUSH_GROUP";
				case GL_DEBUG_TYPE_POP_GROUP: return "GL_DEBUG_TYPE_POP_GROUP";
				case GL_DEBUG_TYPE_OTHER: return "GL_DEBUG_TYPE_OTHER";
				default: return "Unknown GL_DEBUG_TYPE_* enum";
			}
		};

		const auto severityToString = [](GLenum severity) {
			switch (severity) {
				case GL_DEBUG_SEVERITY_LOW: return "GL_DEBUG_SEVERITY_LOW";
				case GL_DEBUG_SEVERITY_MEDIUM: return "GL_DEBUG_SEVERITY_MEDIUM";
				case GL_DEBUG_SEVERITY_HIGH: return "GL_DEBUG_SEVERITY_HIGH";
				case GL_DEBUG_SEVERITY_NOTIFICATION: return "GL_DEBUG_SEVERITY_NOTIFICATION";
				default: return "Unknown GL_DEBUG_SEVERITY_* enum";
			}
		};

		std::cerr
			<< "=====================================\n"
			<< "=== OpenGL Debug Message Callback ===\n"
			<< "=====================================\n"
			<< "id: " << id << "\n"
			<< "source: " << sourceToString(source) << "\n"
			<< "type: " << typeToString(type) << "\n"
			<< "severity: " << severityToString(severity) << "\n"
			<< "message: " << message << "\n\n";
	}
}
