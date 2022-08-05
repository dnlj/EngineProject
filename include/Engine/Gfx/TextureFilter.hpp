#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>


namespace Engine::Gfx {
	enum class TextureFilter : GLenum {
		Nearest,
		Bilinear,
		Trilinear,
		// TODO: Anisotropic
	};
}
