#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Detail/Detail.hpp>

#define ENGINE_DEBUG(msg)\
	Engine::Detail::log(std::clog, "[DEBUG]", __FILE__, __LINE__) << msg << '\n'

namespace Engine::Debug {
	/** 
	 * @brief Checks and prints any OpenGL errors.
	 */
	void checkOpenGLErrors();

	// TODO: Doc
	void checkOpenGLShaderCompilation(GLuint shader);

	/**
	 * @brief Converts an OpenGL error constant to a string.
	 * @param[in] error The error constant.
	 * @return The string representation of the error constant
	 */
	std::string openGLErrorToString(const GLint error);

	// TODO: Doc
	void openGLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
}
