#pragma once

// STD
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/ShaderManager.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/InputEvent.hpp>
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/FontManager.hpp>
#include <Engine/Gui/ShapedString.hpp>
#include <Engine/Gui/Cursor.hpp>
#include <Engine/Gui/Action.hpp>
#include <Engine/Gui/Theme.hpp>

namespace Engine {
	class Camera;
}

namespace Engine::Gui {
	using NativeHandle = void*;

	class Context {
			using ActivateCallback = std::function<void()>;

			// TODO: doc
			using MouseMoveCallback = std::function<void(glm::vec2)>;

			/**
			 * @param panel The panel being activated
			 * @return True if the activation has been intercepted; otherwise false.
			 */
			using PanelBeginActivateCallback = std::function<bool(Panel* panel)>;
			using PanelEndActivateCallback = std::function<void(Panel* panel)>;

			/**
			 * @param text The input text
			 * @return True to consume the input; otherwise false.
			 */
			using TextCallback = std::function<bool(std::string_view text)>;

			using KeyCallback = std::function<bool(Engine::Input::InputEvent)>;

			using PanelUpdateFunc = std::function<void(Panel*)>;

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

			//#define DEBUG_GUI
			struct RenderState {
				#ifdef DEBUG_GUI
				glm::vec4 color = {};
				#endif
				const Panel* current = nullptr; /* The current panel being rendered */
				PanelId id = invalidPanelId; /* The id of the current panel */
				PanelId pid = invalidPanelId; /* The parent id of the current panel */
				int32 layer; /* The layer being rendered */
				glm::vec2 offset; /* The offset to use for rendering */
			};

			struct BFSStateData {
				const Panel* panel;
			};

			struct CursorEntry {
				Panel* panel;
				Cursor cursor;
			};

		private:
			constexpr static PanelId invalidPanelId = -1;

			std::vector<Action> actionQueue;
			NativeHandle nativeHandle = {};
			std::string textBuffer;
			void* userdata = nullptr;

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
			Theme theme;

			std::vector<StringData> stringsToRender;

			/* Scene graph traversal */
			RenderState renderState;
			std::vector<BFSStateData> bfsCurr;
			std::vector<BFSStateData> bfsNext;

			/* Panel state */
			// If you add any more context panel state make sure to update `deletePanel` to remove any references on delete
			Panel* root;
			Panel* active = nullptr;
			Panel* focus = nullptr;
			Panel* hover = nullptr;

			std::vector<Panel*> hoverStack;
			std::vector<Panel*> hoverStackBack;
			bool hoverValid = false;

			std::vector<Panel*> focusStack;
			std::vector<Panel*> focusStackBack;

			glm::vec2 view;
			glm::vec2 cursor = {};

			/* Panel id management */
			FlatHashMap<const Panel*, PanelId> panelIdMap;
			std::vector<PanelId> freePanelIds;
			PanelId nextPanelId = invalidPanelId;

			/* Callbacks */
			FlatHashMap<Panel*, ActivateCallback> activateCallbacks;
			FlatHashMap<Panel*, MouseMoveCallback> mouseMoveCallbacks;
			FlatHashMap<Panel*, PanelBeginActivateCallback> panelBeginActivateCallbacks;
			FlatHashMap<Panel*, PanelEndActivateCallback> panelEndActivateCallbacks;
			FlatHashMap<Panel*, TextCallback> textCallbacks;
			FlatHashMap<Panel*, KeyCallback> keyCallbacks;
			FlatHashMap<Panel*, PanelUpdateFunc> panelUpdateFunc;

			/* Cursors */
			Cursor currentCursor = Cursor::Normal;
			Clock::TimePoint lastBlink = {};
			Clock::Duration cursorBlinkRate = std::chrono::milliseconds{530}; // 530ms = default blink rate on Windows
			int32 activateCount = 0;
			Clock::Duration clickRate = std::chrono::milliseconds{500}; // 500ms = default double click time on Windows
			glm::vec2 clickSize = {4, 4}; // (4, 4) = default double click rect on Windows
			glm::vec2 clickLastPos = {};
			Clock::TimePoint clickLastTime = {};

		public:
			Context(ShaderManager& shaderManager, Camera& camera);
			Context(Context&) = delete;
			~Context();

			ENGINE_INLINE void setUserdata(void* ptr) noexcept { userdata = ptr; }

			template<class T>
			ENGINE_INLINE T* getUserdata() const noexcept { return reinterpret_cast<T*>(userdata); }

			void configUserSettings();

			void render();

			ENGINE_INLINE Panel* getRoot() const noexcept { return root; }

			ENGINE_INLINE auto& getTheme() const noexcept { return theme; }

			ENGINE_INLINE auto getActivateCount() const noexcept {
				return activateCount;
			}

			ENGINE_INLINE void setNativeWindowHandle(const NativeHandle handle) noexcept {
				nativeHandle = handle;
			}

			ENGINE_INLINE bool isBlinking() const noexcept {
				return (((Clock::now() - lastBlink) / cursorBlinkRate) & 1) == 0;
			}

			ENGINE_INLINE void updateBlinkTime() noexcept {
				lastBlink = Clock::now();
			}

			void drawRect(const glm::vec2 pos, const glm::vec2 size, glm::vec4 color);
			void drawString(glm::vec2 pos, const ShapedString* fstr);

			void unsetActive();
			void updateHover();

			ENGINE_INLINE PanelId getPanelId(const Panel* panel) const {
				auto found = panelIdMap.find(panel);
				ENGINE_DEBUG_ASSERT(found != panelIdMap.end(), "Attempting to get id of unregistered Panel.");
				return found->second;
			}
			
			template<class P, class... Args>
			P* constructPanel(Args&&... args) {
				auto p = new P(this, std::forward<Args>(args)...);
				registerPanel(p);
				return p;
			}

			template<class P, class... Args>
			P* createPanel(Panel* parent, Args&&... args) {
				ENGINE_DEBUG_ASSERT(parent != nullptr, "Creating a panel without a parent should be done with `constructPanel`.");
				auto p = constructPanel<P>(std::forward<Args>(args)...);
				parent->addChild(p);
				return p;
			}

			void deletePanel(Panel* panel) {
				if (panel == active) { unsetActive(); }
				if (panel == focus) { setFocus(panel->getParent()); }
				panel->setEnabled(false);
				updateHover();

				deregisterPanelUpdateFunc(panel);
				deregisterActivate(panel);
				deregisterMouseMove(panel);
				deregisterBeginActivate(panel);
				deregisterEndActivate(panel);
				deregisterTextCallback(panel);
				deregisterPanel(panel);
				delete panel;
			}
			
			ENGINE_INLINE void registerPanelUpdateFunc(Panel* panel, PanelUpdateFunc func) {
				ENGINE_DEBUG_ASSERT(!panelUpdateFunc[panel], "Attempting to add duplicate panel update function.");
				panelUpdateFunc[panel] = func;
			}

			ENGINE_INLINE void deregisterPanelUpdateFunc(Panel* panel) {
				panelUpdateFunc.erase(panel);
			}

			ENGINE_INLINE void registerActivate(Panel* panel, ActivateCallback callback) {
				ENGINE_DEBUG_ASSERT(!activateCallbacks[panel], "Attempting to add duplicate activate callback.");
				activateCallbacks[panel] = callback;
			}

			ENGINE_INLINE void deregisterActivate(Panel* panel) {
				activateCallbacks.erase(panel);
			}

			ENGINE_INLINE void registerMouseMove(Panel* panel, MouseMoveCallback callback) {
				ENGINE_DEBUG_ASSERT(!mouseMoveCallbacks[panel], "Attempting to add duplicate mouse move callback.");
				mouseMoveCallbacks[panel] = callback;
			}

			ENGINE_INLINE void deregisterMouseMove(Panel* panel) {
				mouseMoveCallbacks.erase(panel);
			}

			ENGINE_INLINE void registerBeginActivate(Panel* panel, PanelBeginActivateCallback callback) {
				ENGINE_DEBUG_ASSERT(!panelBeginActivateCallbacks[panel], "Attempting to add duplicate panel activate callback.");
				panelBeginActivateCallbacks[panel] = callback;
			}
			
			ENGINE_INLINE void deregisterBeginActivate(Panel* panel) {
				panelBeginActivateCallbacks.erase(panel);
			}

			ENGINE_INLINE void registerEndActivate(Panel* panel, PanelEndActivateCallback callback) {
				ENGINE_DEBUG_ASSERT(!panelEndActivateCallbacks[panel], "Attempting to add duplicate panel activate callback.");
				panelEndActivateCallbacks[panel] = callback;
			}
			
			ENGINE_INLINE void deregisterEndActivate(Panel* panel) {
				panelBeginActivateCallbacks.erase(panel);
			}
			
			ENGINE_INLINE void registerTextCallback(Panel* panel, TextCallback callback) {
				ENGINE_DEBUG_ASSERT(!textCallbacks[panel], "Attempting to add duplicate char callback.");
				textCallbacks[panel] = callback;
			}
			
			ENGINE_INLINE void deregisterTextCallback(Panel* panel) {
				textCallbacks.erase(panel);
			}
			
			ENGINE_INLINE auto getCursor() const noexcept {
				return cursor;
			}

			ENGINE_INLINE void setCursor(Cursor cursor) {
				if (cursor == currentCursor) { return; }
				currentCursor = cursor;
				updateCursor();
			}

			void updateCursor();

			/**
			 * Sets the position of the Input Method Editor.
			 * Requires that setNativeWindowHandle has been called.
			 */
			void setIMEPosition(const glm::vec2 pos);

			/**
			 * Gets the most hovered panel.
			 */
			ENGINE_INLINE Panel* getHover() noexcept { return hover; }
			ENGINE_INLINE const Panel* getHover() const noexcept { return hover; }

			/**
			 * Set the focused panel.
			 */
			void setFocus(Panel* panel);

			/**
			 * Gets the most focused panel.
			 */
			ENGINE_INLINE Panel* getFocus() noexcept { return focus; }
			ENGINE_INLINE const Panel* getFocus() const noexcept { return focus; }
			
			/**
			 * Gets the active panel.
			 */
			ENGINE_INLINE Panel* getActive() noexcept { return active; }
			ENGINE_INLINE const Panel* getActive() const noexcept { return active; }

			void queueAction(Action act);

			/**
			 * Copies the UTF-8 encoded text to the clipboard.
			 * Requires that setNativeWindowHandle has been called.
			 */
			void setClipboard(std::string_view view);

			/**
			 * Gets the text currently in the clipboard as a UTF-8 encoded string.
			 */
			std::string getClipboardText() const;

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
			bool onText(std::string_view str);

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

			ENGINE_INLINE void registerPanel(const Panel* panel) {
				panelIdMap[panel] = claimNextPanelId();
			}

			ENGINE_INLINE void deregisterPanel(const Panel* panel) {
				ENGINE_DEBUG_ASSERT(panel != nullptr, "Attempting to deregister nullptr.");
				auto found = panelIdMap.find(panel);
				ENGINE_DEBUG_ASSERT(found != panelIdMap.end(), "Attempting to deregister an unregistered Panel (", panel, ").");
				freePanelIds.push_back(found->second);
				panelIdMap.erase(found);
			}

			/**
			 * Adds the glyphs needed to draw the string to the glyph vertex buffer.
			 */
			void renderString(const ShapedString& str, PanelId parent, glm::vec2 base, FontGlyphSet* font);
	};
}
