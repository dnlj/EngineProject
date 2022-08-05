#pragma once

// GLM
#include <glm/vec4.hpp>

// OpenGL
#include <glloadgen/gl_core_4_5.hpp>


namespace Engine::Gfx {
	enum class TextureFormat : GLenum {
		NONE = 0,
		R16F = GL_R16F,
		R32F = GL_R32F,

		R8 = GL_R8,
		RGB8 = GL_RGB8,
		RGBA8 = GL_RGBA8,

		SRGB8 = GL_SRGB8,
		SRGBA8 = GL_SRGB8_ALPHA8, // sRGB w/ linear alpha. 8 bits per channel.

		R8U = GL_R8UI,
		R16U = GL_R16UI,
		R32U = GL_R32UI,
	};
}
