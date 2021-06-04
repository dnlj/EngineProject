#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class ParallaxBackgroundSystem : public System {
		private:
			GLuint vao = 0;
			GLuint vbo = 0;
			Engine::Shader shader;
			Engine::TextureRef texture;

			constexpr static GLuint dataBindingIndex = 0;

		public:
			ParallaxBackgroundSystem(SystemArg arg);
			~ParallaxBackgroundSystem();
			void render(const RenderLayer layer);
	};
}
