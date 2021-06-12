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

			struct Vertex {
				glm::vec4 color;
				glm::vec2 pos;
			}; static_assert(sizeof(Vertex) == sizeof(GLfloat) * 6);

		private:
			std::vector<Panel*> focusStack;
			std::vector<Vertex> verts;
			std::vector<BFSStateData> bfsCurr;
			std::vector<BFSStateData> bfsNext;

			constexpr static GLuint vertBindingIndex = 0;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLsizei vboCapacity = 0;
			Shader shader;
			glm::vec4 color = {1.0f, 0.0f, 0.0f, 0.2f};

			glm::vec2 view;
			glm::vec2 offset;
			glm::vec2 cursor = {};

			Panel* root;
			bool focusValid = false;

		public:
			// TODO: split shader into own class so we dont depend on engine
			Context(Engine::EngineInstance& engine);
			~Context();
			void render();
			void addRect(const glm::vec2 pos, const glm::vec2 size);
			void updateFocus();

			/**
			 * Gets the most focused panel.
			 */
			ENGINE_INLINE Panel* getFocus() noexcept { return focusStack.empty() ? nullptr : focusStack.back(); }
			ENGINE_INLINE const Panel* getFocus() const noexcept { return const_cast<Context*>(this)->getFocus(); }

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
