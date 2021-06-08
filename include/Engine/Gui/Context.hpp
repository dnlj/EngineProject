#pragma once

// STD
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/EngineInstance.hpp>


namespace Engine::Gui {
	class Context {
		private:
			std::vector<glm::vec2> verts;

			constexpr static GLuint vertBindingIndex = 0;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLsizei vboCapacity = 0;
			Shader shader;

			glm::vec2 view;

		public:
			// TODO: split shader into own class so we dont depend on engine
			Context(Engine::EngineInstance& engine);
			~Context();
			void render();
			void addRect(const glm::vec2 pos, const glm::vec2 size);
			// TODO: inputs
	};
}
