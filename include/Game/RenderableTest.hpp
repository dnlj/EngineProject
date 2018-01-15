#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/TextureManager.hpp>

namespace Game {
	class RenderableTest {
		public:
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint shader = 0;
			GLuint texture = 0;

			~RenderableTest();
			void setup(Engine::TextureManager& textureManager);
	};
}