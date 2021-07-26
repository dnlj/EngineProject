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
#include <Engine/Gui/FontManager.hpp>
#include <Engine/Gui/ShapedString.hpp>


namespace Engine::Gui {
	class Context {
		private:
			// There is no point in using integers here because
			// of GLSL integer requirement range limitations. In short
			// GLSL integers may be implemented as floats with a limited range.
			// See GLSL spec for details (4.5.2  Precision Qualifiers)
			using PanelId = float32;
			
			struct PolyDrawGroup {
				int32 offset;
				int32 count;
			};

			struct PolyVertex {
				glm::vec4 color;
				glm::vec2 pos;
				PanelId id;
				PanelId pid;
			}; static_assert(sizeof(PolyVertex) == sizeof(GLfloat) * 6 + sizeof(PanelId) * 2);

			struct GlyphDrawGroup {
				int32 layer;
				int32 offset;
				int32 count;
				FontGlyphSet* glyphSet;
			};

			struct GlyphVertex {
				glm::vec2 pos;
				uint32 index; // TODO: uint16?
				PanelId parent;
			}; static_assert(sizeof(GlyphVertex) == sizeof(glm::vec2) + sizeof(uint32) + sizeof(PanelId));

			struct StringData {
				int32 layer;
				PanelId parent;
				glm::vec2 pos;
				const ShapedString* str;
			};
			
			struct RenderState {
				glm::vec4 color = {1.0f, 0.0f, 0.0f, 0.2f};
				const Panel* current = nullptr; /* The current panel being rendered */
				PanelId id = invalidPanelId; /* The id of the current panel */
				PanelId pid = invalidPanelId; /* The parent id of the current panel */
				int32 layer; /* The layer being rendered */
				glm::vec2 offset; /* The offset to use for rendering */
			};

			struct BFSStateData {
				glm::vec2 offset;
				const Panel* panel;
			};

		private:
			constexpr static PanelId invalidPanelId = -1;

			/* Main framebuffer and clipping */
			GLuint fbo = 0;
			Texture2D colorTex;
			Texture2D clipTex1;
			Texture2D clipTex2;
			GLenum activeClipTex = 1;
			GLuint quadVAO;
			GLuint quadVBO;
			ShaderRef quadShader;

			/* Polygon members */
			GLuint polyVAO = 0;
			GLuint polyVBO = 0;
			GLsizei polyVBOCapacity = 0;
			std::vector<PolyDrawGroup> polyDrawGroups;
			std::vector<PolyVertex> polyVertexData;
			ShaderRef polyShader;

			/* Glyph members */
			GLuint glyphVAO = 0;
			GLuint glyphVBO = 0;
			GLsizei glyphVBOCapacity = 0;
			std::vector<GlyphDrawGroup> glyphDrawGroups;
			std::vector<GlyphVertex> glyphVertexData;
			ShaderRef glyphShader;

			/* Text rendering helpers */
			FontManager fontManager;
			FontId fontId_a; // TODO: rm when done testing
			FontId fontId_b; // TODO: rm when done testing
			std::vector<StringData> stringsToRender;

			/* Scene graph traversal */
			RenderState renderState;
			std::vector<BFSStateData> bfsCurr;
			std::vector<BFSStateData> bfsNext;

			/* Panel state */
			Panel* root;
			Panel* active = nullptr;
			std::vector<Panel*> hoverStack;
			bool hoverValid = false;
			glm::vec2 view;
			glm::vec2 cursor = {};

			/* Panel id management */
			FlatHashMap<const Panel*, PanelId> panelIdMap;
			std::vector<PanelId> freePanelIds;
			PanelId nextPanelId = invalidPanelId;

		public:
			// TODO: split shader into own class so we dont depend on engine
			Context(Engine::EngineInstance& engine);
			~Context();
			void render();

			void drawRect(const glm::vec2 pos, const glm::vec2 size);
			void drawString(glm::vec2 pos, const ShapedString* fstr);

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

			/**
			 * Adds the glyphs needed to draw the string to the glyph vertex buffer.
			 */
			void renderString(const ShapedString& str, PanelId parent, glm::vec2 base, FontGlyphSet* font);
	};
}
