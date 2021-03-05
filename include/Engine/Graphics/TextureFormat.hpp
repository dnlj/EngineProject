#pragma once

// GLM
#include <glm/vec4.hpp>

// OpenGL
#include <glloadgen/gl_core_4_5.hpp>


namespace Engine {
	enum class TextureFormat : GLenum {
		NONE = 0,
		RGB8 = GL_RGB8,
		RGBA8 = GL_RGBA8,
		SRGB8 = GL_SRGB8,
		SRGBA8 = GL_SRGB8_ALPHA8, // sRGB w/ linear alpha. 8 bits per channel.
	};
}
