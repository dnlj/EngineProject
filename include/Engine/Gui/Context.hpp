#pragma once

// STD
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/Gui/Panel.hpp>
#include <Engine/Input/InputEvent.hpp>


namespace Engine::Gui {
	class Context {
		private:
			struct BFSStateData {
				glm::vec2 offset;
				const Panel* panel;
			};

		private:
			std::vector<glm::vec2> verts;
			std::vector<BFSStateData> bfsCurr;
			std::vector<BFSStateData> bfsNext;

			constexpr static GLuint vertBindingIndex = 0;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLsizei vboCapacity = 0;
			Shader shader;

			glm::vec2 view;
			glm::vec2 offset;

			Panel* root;

		public:
			// TODO: split shader into own class so we dont depend on engine
			Context(Engine::EngineInstance& engine);
			~Context();
			void render();
			void addRect(const glm::vec2 pos, const glm::vec2 size);

			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onMouse(const Engine::Input::InputEvent event);

			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onMouseMove(const Engine::Input::InputEvent event);

			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onMouseWheel(const Engine::Input::InputEvent event);
			
			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onKey(const Engine::Input::InputEvent event);
			
			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onChar(const wchar_t ch);// TODO: How to support full unicode and "Win + ." emoji picker? Look into WM_IME_* messages

			void onResize(const int32 w, const int32 h);
			void onFocus(const bool has);
	};
}
