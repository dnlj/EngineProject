#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>


namespace Engine {
	enum class TextureFilter : GLenum {
		NEAREST,
		BILINEAR,
		TRILINEAR,
		// TODO: Anisotropic
	};
}
