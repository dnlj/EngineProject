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
#include <Engine/ArrayView.hpp>

namespace Engine {
	class Camera;
}

namespace Engine::Gui {
	using NativeHandle = void*;

	class Context {
			// TODO: doc
			using MouseMoveCallback = std::function<void(glm::vec2)>;

			/**
			 * @param text The input text
			 * @return True to consume the input; otherwise false.
			 */
			using TextCallback = std::function<bool(std::string_view text)>;

			using KeyCallback = std::function<bool(Engine::Input::InputEvent)>;

			using PanelUpdateFunc = std::function<void(Panel*)>;

		private:
			struct PolyDrawGroup {
				int32 zindex = {};
				int32 offset = {};
				int32 count = {};
				Bounds clip = {};
				TextureHandle2D tex = {};
			};

			struct PolyVertex {
				glm::vec4 color;
				glm::vec2 texCoord;
				glm::vec2 pos;
			}; static_assert(sizeof(PolyVertex) == sizeof(GLfloat) * 8);

			struct GlyphDrawGroup {
				int32 zindex = {};
				int32 offset = {};
				int32 count = {};
				Bounds clip = {};
				Font font = {};
			};

			struct GlyphVertex {
				glm::vec2 pos;
				uint32 index;
			}; static_assert(sizeof(GlyphVertex) == sizeof(glm::vec2) + sizeof(uint32));

			struct CursorEntry {
				Panel* panel;
				Cursor cursor;
			};

		private:
			// TODO: Should action events instead be associated with a panel instead of having multiple queues?
			std::vector<ActionEvent> focusActionQueue;
			std::vector<ActionEvent> hoverActionQueue;

			NativeHandle nativeHandle = {};
			std::string textBuffer;
			void* userdata = nullptr;

			/* Main framebuffer and clipping */
			GLuint fbo = 0;
			Texture2D colorTex;
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
			
			GLuint polyEBO = 0;
			GLsizei polyEBOCapacity = 0;
			std::vector<uint16> polyElementData;

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

			/* Render state */
			std::vector<Bounds> clipStack; // TODO: should be part of render state?
			TextureHandle2D activeTexture;
			Texture2D defaultTexture; /** Default blank (white) texture */
			glm::vec2 drawOffset; /* The offset to use for rendering */

			struct RenderState {
				Font font = nullptr;
				int32 zindex = -1;
			} renderState;

			/* Panel state */
			// If you add any more context panel state make sure to update `deletePanel` to remove any references on delete
			Panel* root = nullptr;
			Panel* active = nullptr;
			Panel* focus = nullptr;
			Panel* hover = nullptr;

			std::vector<Panel*> hoverStack;
			std::vector<Panel*> hoverStackBack;
			bool hoverValid = false;

			std::vector<Panel*> focusStack;
			std::vector<Panel*> focusStackBack;

			glm::vec2 view = {};
			glm::vec2 cursor = {};

			/* Callbacks */
			FlatHashMap<Panel*, MouseMoveCallback> mouseMoveCallbacks;
			FlatHashMap<Panel*, TextCallback> textCallbacks;
			FlatHashMap<Panel*, KeyCallback> keyCallbacks;

			int currPanelUpdateFunc = 0;
			struct PanelUpdatePair { Panel* panel; PanelUpdateFunc func; };
			std::vector<PanelUpdatePair> panelUpdateFunc;

			/* Cursors */
			Cursor currentCursor = Cursor::Normal;
			Clock::TimePoint lastBlink = {};
			Clock::Duration cursorBlinkRate = std::chrono::milliseconds{530}; // 530ms = default blink rate on Windows
			int32 activateCount = 0;
			Clock::Duration clickRate = std::chrono::milliseconds{500}; // 500ms = default double click time on Windows
			glm::vec2 clickSize = {4, 4}; // (4, 4) = default double click rect on Windows
			glm::vec2 clickLastPos = {};
			Clock::TimePoint clickLastTime = {};

			/* Other metrics */
			/**
			 * The resizable area around a window.
			 *
			 * Ideally we would query SM_C(X/Y)SIZEFRAME but these return incorrect values.
			 * With GetSystemMetrics(ForDpi) the size frame is always 4 regardless of dpi, and scale.
			 * 
			 * The same issue is present with SPI_GETNONCLIENTMETRICS and SystemParametersInfo(ForDPI).
			 */
			constexpr static glm::vec2 resizeBorderSize = {7, 7}; // TODO: adjust for DPI aware - WM_DPICHANGED

			float32 scrollChars = 3; // 3 = default on Windows
			float32 scrollLines = 3; // 3 = default on Windows

		public:
			Context(ShaderManager& shaderManager, TextureManager& textureManager, Camera& camera);
			Context(Context&) = delete;
			~Context();

			ENGINE_INLINE constexpr static auto getResizeBorderSize() noexcept { return resizeBorderSize; }
			ENGINE_INLINE float32 getScrollChars() const noexcept { return scrollChars; }
			ENGINE_INLINE float32 getScrollLines() const noexcept { return scrollLines; }

			ENGINE_INLINE void setUserdata(void* ptr) noexcept { userdata = ptr; }

			template<class T>
			ENGINE_INLINE T* getUserdata() const noexcept { return reinterpret_cast<T*>(userdata); }

			void configUserSettings();

			void render();

			ENGINE_INLINE Panel* getRoot() const noexcept { return root; }
			ENGINE_INLINE auto& getTheme() const noexcept { return theme; }
			ENGINE_INLINE auto getActivateCount() const noexcept { return activateCount; }

			ENGINE_INLINE void setNativeWindowHandle(const NativeHandle handle) noexcept {
				nativeHandle = handle;
				configUserSettings();
			}

			ENGINE_INLINE bool isBlinking() const noexcept {
				return (((Clock::now() - lastBlink) / cursorBlinkRate) & 1) == 0;
			}

			ENGINE_INLINE void updateBlinkTime() noexcept {
				lastBlink = Clock::now();
			}

			void flushDrawBuffer();
			void resetDraw();
			void nextDrawGroupPoly();
			void nextDrawGroupGlyph();
			void pushClip(Bounds bounds);
			void popClip();

			void drawTexture(TextureHandle2D tex, glm::vec2 pos, glm::vec2 size);

			/**
			 * Draws a convex polygon from a ordered set of perimeter points.
			 * If the points are not in order the results are undefined.
			 * 
			 * @param points Three or more ordered perimeter points.
			 * @param color The color of the polygon.
			 */
			void drawPoly(ArrayView<const glm::vec2> points, glm::vec4 color);

			/**
			 * Draws a rectangle from a position and size.
			 */
			void drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color);

			/**
			 * Draws a line between two points.
			 */
			void drawLine(glm::vec2 a, glm::vec2 b, float32 width, glm::vec4 color);

			void drawString(glm::vec2 pos, const ShapedString* fstr);

			void unsetActive();
			void updateHover();

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

			/**
			 * Delete a panel.
			 * @param panel The panel to delete.
			 * @param isChild Internal parameter used for recursive child deletion. Leave at default.
			 */
			void deletePanel(Panel* panel, bool isChild = false);

			// TODO: rename add since we have multiple now
			ENGINE_INLINE void addPanelUpdateFunc(Panel* panel, PanelUpdateFunc func) {
				panelUpdateFunc.push_back({.panel = panel, .func = func});
			}

			ENGINE_INLINE void clearPanelUpdateFuncs(Panel* panel) {
				for (auto i=std::ssize(panelUpdateFunc)-1; i >= 0; --i) {
					if (panelUpdateFunc[i].panel == panel) {
						if (i <= currPanelUpdateFunc) { --currPanelUpdateFunc; }
						panelUpdateFunc.erase(panelUpdateFunc.begin() + i);
					}
				}
			}

			ENGINE_INLINE void registerMouseMove(Panel* panel, MouseMoveCallback callback) {
				ENGINE_DEBUG_ASSERT(!mouseMoveCallbacks[panel], "Attempting to add duplicate mouse move callback.");
				mouseMoveCallbacks[panel] = callback;
			}

			ENGINE_INLINE void deregisterMouseMove(Panel* panel) {
				mouseMoveCallbacks.erase(panel);
			}

			ENGINE_INLINE void registerTextCallback(Panel* panel, TextCallback callback) {
				ENGINE_DEBUG_ASSERT(!textCallbacks[panel], "Attempting to add duplicate char callback.");
				textCallbacks[panel] = callback;
			}
			
			ENGINE_INLINE void deregisterTextCallback(Panel* panel) {
				textCallbacks.erase(panel);
			}

			// TODO: rename - getCursorPos to avoid confusion with setCursor, maybe rename setCursor to setCursorStyle or similar to further disambiguate.
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

			/**
			 * Queue an action for the focused panel.
			 */
			void queueFocusAction(Action action, Input::Value value = {}) { focusActionQueue.push_back({action, value}); }

			/**
			 * Queue an action for the hovered panel.
			 */
			void queueHoverAction(Action action, Input::Value value = {}) { hoverActionQueue.push_back({action, value}); }

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
			 * Changes the hovered panel to be focused.
			 */
			void focusHover();

			bool onActivate(const bool state, Clock::TimePoint time);

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
			ENGINE_INLINE void registerPanel(const Panel* panel) {
			}

			ENGINE_INLINE void deregisterPanel(const Panel* panel) {
				ENGINE_DEBUG_ASSERT(panel != nullptr, "Attempting to deregister nullptr.");
			}
			
			ENGINE_INLINE void drawVertex(glm::vec2 pos, glm::vec2 texCoord, glm::vec4 color = {1,1,1,1}) {
				polyVertexData.push_back({
					.color = color,
					.texCoord = texCoord,
					.pos = pos + drawOffset,
				});
			}

			ENGINE_INLINE void drawVertex(glm::vec2 pos, glm::vec4 color) {
				drawVertex(pos, {}, color);
			}

			ENGINE_INLINE void drawTri(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec4 color) {
				drawVertex(a, color); drawVertex(b, color); drawVertex(c, color);
			}

			ENGINE_INLINE void addPolyElements(uint32 i1, uint32 i2, uint32 i3) {
				polyElementData.push_back(i1); polyElementData.push_back(i2); polyElementData.push_back(i3);
			}
	};
}
