#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.h>

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/TextureManager.hpp>

namespace Game {
	class RenderableTest {
		public:
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint shader = 0;
			GLuint texture = 0;
			b2Body* body = nullptr;

			~RenderableTest();
			void setup(Engine::TextureManager& textureManager, b2World& world);
	};
}