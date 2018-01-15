#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

namespace Engine::Debug::GL {
	/**
	 * @brief Converts an OpenGL error enum to a string.
	 * @param[in] error The enum.
	 * @return The string representation of the enum.
	 */
	std::string errorEnumToString(GLenum error);

	/**
	 * @brief Converts an OpenGL source enum to a string.
	 * @param[in] source The enum.
	 * @return The string representation of the enum.
	 */
	std::string sourceEnumToString(GLenum source);

	/**
	 * @brief Converts an OpenGL type enum to a string.
	 * @param[in] type The enum.
	 * @return The string representation of the enum.
	 */
	std::string typeEnumToString(GLenum type);

	/**
	 * @brief Converts an OpenGL severity enum to a string.
	 * @param[in] severity The enum.
	 * @return The string representation of the enum.
	 */
	std::string severityEnumToString(GLenum severity);

	/**
	 * @brief A function to use with glDebugMessageCallback.
	 * See OpenGL documentation for glDebugMessageCallback.
	 */
	void debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
}