#pragma once

// STD
#include <string>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Resource.hpp>


namespace Game {
	class SpriteComponent {
		public:
			// TODO: Texture resource instead of Resource<T> ?
			Engine::Resource<GLuint> texture;
	};
}
