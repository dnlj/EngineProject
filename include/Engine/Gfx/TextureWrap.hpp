#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>


namespace Engine {
	enum class TextureWrap : GLenum {
		Repeat = GL_REPEAT,
		MirrorRepeat = GL_MIRRORED_REPEAT,
		ClampToEdge = GL_CLAMP_TO_EDGE,
	};
}
