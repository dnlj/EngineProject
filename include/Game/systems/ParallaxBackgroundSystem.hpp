#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/TextureManager.hpp>
#include <Engine/ShaderManager.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class ParallaxBackgroundSystem : public System {
		private:
			struct InstData {
				glm::vec2 scale;
				GLfloat xoff;
			}; static_assert(sizeof(InstData) == 3 * sizeof(GLfloat), "Assumed to be tightly packed");

			struct Layer {
				Engine::TextureRef texture;
				float32 speedScale;
			};

		private:
			GLuint vao = 0;
			GLuint vbo = 0;
			Engine::ShaderRef shader;

			std::vector<InstData> instData;
			std::vector<Layer> layers;

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
			void run(const float32 dt);
			void render(const RenderLayer layer);
	};
}
