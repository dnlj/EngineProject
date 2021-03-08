#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>


namespace Engine {
	enum class TextureWrap : GLenum {
		REPEAT = GL_REPEAT,
		MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
		CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
	};
}
