#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class ParallaxBackgroundSystem : public System {
		private:
			struct InstData {
				glm::vec2 scale;
				GLfloat xoff;
			}; static_assert(sizeof(InstData) == 3 * sizeof(GLfloat), "Assumed to be tightly packed");

		private:
			GLuint vao = 0;
			GLuint vbo = 0;
			Engine::Shader shader;
			Engine::TextureRef texture;

			constexpr static GLuint rectBindingIndex = 0;
			constexpr static GLuint instBindingIndex = 1;
			constexpr static glm::vec2 rect[] = {
				{1.0f, 1.0f},
				{-1.0f, 1.0f},
				{-1.0f, -1.0f},

				{-1.0f, -1.0f},
				{1.0f, -1.0f},
				{1.0f, 1.0f},
			};

		public:
			ParallaxBackgroundSystem(SystemArg arg);
			~ParallaxBackgroundSystem();
			void render(const RenderLayer layer);
	};
}
