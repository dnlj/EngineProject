// STD
#include <iostream>
#include <string>

// Engine
#include <Engine/Debug/GL/GL.hpp>
#include <Engine/Engine.hpp>

namespace Engine::Debug::GL {
	std::string errorEnumToString(GLenum error) {
		switch (error) {
			case GL_NO_ERROR: return "GL_NO_ERROR";
			case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
			case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
			case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
			case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
			case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
			case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
			case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
		}

		const auto msg = "Unknown GL_* enum (" + std::to_string(error) + ")";
		ENGINE_WARN(msg);
		return msg;
	}

	std::string sourceEnumToString(GLenum source) {
		switch (source) {
			case GL_DEBUG_SOURCE_API: return "GL_DEBUG_SOURCE_API";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
			case GL_DEBUG_SOURCE_SHADER_COMPILER: return "GL_DEBUG_SOURCE_SHADER_COMPILER";
			case GL_DEBUG_SOURCE_THIRD_PARTY: return "GL_DEBUG_SOURCE_THIRD_PARTY";
			case GL_DEBUG_SOURCE_APPLICATION: return "GL_DEBUG_SOURCE_APPLICATION";
			case GL_DEBUG_SOURCE_OTHER: return "GL_DEBUG_SOURCE_OTHE";
		}

		const auto msg = "Unknown GL_DEBUG_SOURCE_* enum (" + std::to_string(source) + ")";
		ENGINE_WARN(msg);
		return msg;
	}

	std::string typeEnumToString(GLenum type) {
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
		}

		const auto msg = "Unknown GL_DEBUG_TYPE_* enum (" + std::to_string(type) + ")";
		ENGINE_WARN(msg);
		return msg;
	}

	std::string severityEnumToString(GLenum severity) {
		switch (severity) {
			case GL_DEBUG_SEVERITY_LOW: return "GL_DEBUG_SEVERITY_LOW";
			case GL_DEBUG_SEVERITY_MEDIUM: return "GL_DEBUG_SEVERITY_MEDIUM";
			case GL_DEBUG_SEVERITY_HIGH: return "GL_DEBUG_SEVERITY_HIGH";
			case GL_DEBUG_SEVERITY_NOTIFICATION: return "GL_DEBUG_SEVERITY_NOTIFICATION";
		}

		const auto msg = "Unknown GL_DEBUG_SEVERITY_* enum (" + std::to_string(severity) + ")";
		ENGINE_WARN(msg);
		return msg;
	}

	void debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
		// Skip notifications
		if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) { return; }

		// Output message
		std::cerr
			<< "=====================================\n"
			<< "=== OpenGL Debug Message Callback ===\n"
			<< "=====================================\n"
			<< "id: " << id << "\n"
			<< "source: " << sourceEnumToString(source) << "\n"
			<< "type: " << typeEnumToString(type) << "\n"
			<< "severity: " << severityEnumToString(severity) << "\n"
			<< "message: " << message << "\n\n";

		__debugbreak();
	}
}
