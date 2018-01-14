#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

namespace Engine::Debug::GL {
	/**
	 * @brief Converts an OpenGL error enum to a string.
	 * @param[in] error The error enum.
	 * @return The string representation of the error.
	 */
	std::string errorEnumToString(GLenum error);

	// TODO: Doc
	void debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
}