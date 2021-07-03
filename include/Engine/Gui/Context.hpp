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
#include <Engine/FlatHashMap.hpp>


namespace Engine::Gui {
	class Context {
		private:
			using PanelId = float32; // TODO: reall would like uint16
			
			struct BFSStateData {
				glm::vec2 offset;
				const Panel* panel;
			};

			struct Vertex {
				glm::vec4 color;
				glm::vec2 pos;
				PanelId id;
				PanelId pid;
			}; static_assert(sizeof(Vertex) == sizeof(GLfloat) * 6 + sizeof(PanelId) * 2);
			
			struct MultiDrawData {
				std::vector<GLint> first;
				std::vector<GLsizei> count;
			};

			struct CharData {
				Texture2D tex;
				glm::vec2 size;
				glm::vec2 bearing;
				float32 advance;
			};

			struct GlyphData {
				// Make sure to consider GLSL alignment rules
				glm::vec2 size;
				float32 _size_padding[2];

				glm::vec3 offset;
				float32 _offset_padding[1];
			}; static_assert(sizeof(GlyphData) == sizeof(float32) * 8);

			struct GlyphMetrics {
				// These are both for horizontal layout. For vertical layout we would need separate fields.
				glm::vec2 bearing;
				float32 advance;
				uint32 index;
			};

			struct GlyphVertex {
				glm::vec2 pos;
				uint32 index; // TODO: uint16?
			}; static_assert(sizeof(GlyphVertex) == sizeof(glm::vec2) + sizeof(uint32));

		private:
			std::vector<Panel*> hoverStack;
			std::vector<Vertex> verts;
			MultiDrawData multiDrawData;
			std::vector<BFSStateData> bfsCurr;
			std::vector<BFSStateData> bfsNext;

			constexpr static GLuint vertBindingIndex = 0;
			GLuint fbo = 0;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLsizei vboCapacity = 0;
			ShaderRef shader;
			Texture2D colorTex;
			Texture2D clipTex1;
			Texture2D clipTex2;
			GLenum activeClipTex = 0;

			GLuint glyphSSBO = 0;
			GLsizei glyphSSBOSize = 0;
			FlatHashMap<uint8, uint32> charToIndex;
			std::vector<GlyphData> glyphData;
			std::vector<GlyphMetrics> glyphMetrics;
			std::vector<GlyphVertex> glyphVertexData;
			ShaderRef glyphShader;
			Texture2D glyphTex;
			glm::vec2 maxFace;
			glm::ivec2 indexBounds;
			int nextGlyphIndex = 0; // TODO: glyph index recycling

			// TODO: rm when done with testing
			GLuint glyphVBO = 0;
			GLuint glyphVAO = 0;
			GLsizei glyphVBOSize = 0;
			void renderText2(const std::string_view view, glm::vec2 base); 

			struct {
				glm::vec4 color = {1.0f, 0.0f, 0.0f, 0.2f};
				const Panel* current = nullptr;
			} currRenderState;

			ShaderRef quadShader;
			GLuint quadVAO;
			GLuint quadVBO;

			glm::vec2 view;
			glm::vec2 offset;
			glm::vec2 cursor = {};

			Panel* root;
			Panel* active = nullptr;
			bool hoverValid = false;

			constexpr static PanelId invalidPanelId = -1;//std::numeric_limits<PanelId>::max();
			FlatHashMap<const Panel*, PanelId> panelIdMap;
			std::vector<PanelId> freePanelIds;
			PanelId nextPanelId = invalidPanelId;

		public:
			// TODO: split shader into own class so we dont depend on engine
			Context(Engine::EngineInstance& engine);
			~Context();
			void render();
			void addRect(const glm::vec2 pos, const glm::vec2 size);
			void updateHover();

			ENGINE_INLINE PanelId getPanelId(const Panel* panel) const {
				auto found = panelIdMap.find(panel);
				ENGINE_DEBUG_ASSERT(found != panelIdMap.end(), "Attempting to get id of unregistered Panel.");
				return found->second;
			}

			ENGINE_INLINE void registerPanel(const Panel* panel) {
				panelIdMap[panel] = claimNextPanelId();
				ENGINE_LOG("Panel Id ", panelIdMap[panel], " = ", panel);
			}

			// TODO: name?
			ENGINE_INLINE void deregisterPanel(const Panel* panel) {
				ENGINE_DEBUG_ASSERT(panel != nullptr, "Attempting to deregister nullptr.");
				auto found = panelIdMap.find(panel);
				ENGINE_DEBUG_ASSERT(found != panelIdMap.end(), "Attempting to deregister an unregistered Panel.");
				freePanelIds.push_back(found->second);
				panelIdMap.erase(found);
			}

			/**
			 * Checks if we are hovering any panel.
			 */
			ENGINE_INLINE bool isHoverAny() const noexcept { return !hoverStack.empty(); }

			/**
			 * Gets the most hovered panel.
			 */
			ENGINE_INLINE Panel* getHover() noexcept { return isHoverAny() ? hoverStack.back() : nullptr; }
			ENGINE_INLINE const Panel* getHover() const noexcept { return const_cast<Context*>(this)->getHover(); }

			/**
			 * Gets the most focused panel.
			 */
			ENGINE_INLINE Panel* getFocus() noexcept { return getHover(); } // TODO: actual focus logic.
			ENGINE_INLINE const Panel* getFocus() const noexcept { return getHover(); }
			
			/**
			 * Gets the active panel.
			 */
			ENGINE_INLINE Panel* getActive() noexcept { return active; }
			ENGINE_INLINE const Panel* getActive() const noexcept { return active; }

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

		private:
			[[nodiscard]]
			PanelId claimNextPanelId() {
				if (!freePanelIds.empty()) {
					const auto id = freePanelIds.back();
					return freePanelIds.pop_back(), id;
				}
				return ++nextPanelId;
			}
	};
}
