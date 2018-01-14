#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Detail/Detail.hpp>
#include <Engine/Debug/GL/GL.hpp>

#define ENGINE_DEBUG(msg)\
	Engine::Detail::log(std::clog, "[DEBUG]", __FILE__, __LINE__) << msg << '\n'

namespace Engine::Debug {
}
