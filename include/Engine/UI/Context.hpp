#pragma once

// STD
#include <vector>

// glLoadGen
//#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/InputEvent.hpp>
#include <Engine/UI/Action.hpp>
#include <Engine/UI/Cursor.hpp>
#include <Engine/UI/DrawBuilder.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/UI/Theme.hpp>


namespace Engine {
	class Camera;
}

namespace Engine::UI {
	using NativeHandle = void*;

	class Context : public DrawBuilder {
		public:
			using MouseMoveCallback = std::function<void(glm::vec2)>;

			/**
			 * @param text The input text
			 * @return True to consume the input; otherwise false.
			 */
			using TextCallback = std::function<bool(std::string_view text, Input::KeyCode code)>;

			using KeyCallback = std::function<bool(Input::InputEvent)>;

			using PanelUpdateFunc = std::function<void(Panel*)>;
			using TimerUpdateFunc = std::function<void()>;
			class PanelUpdateFuncId {
				private:
					friend Context;
					uint64 id = 0;
				public:
					PanelUpdateFuncId() = default;
					ENGINE_INLINE constexpr operator bool() const noexcept { return id; }
					ENGINE_INLINE constexpr bool operator==(const PanelUpdateFuncId& other) const noexcept { return id == other.id; }
			};

		private:
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
			Gfx::Texture2D colorTex;
			GLuint quadVAO;
			GLuint quadVBO;
			Gfx::ShaderRef quadShader;

			/* Text rendering helpers */
			Theme theme;

			/* Panel state */
			// If you add any more context panel state make sure to update `deletePanel` to remove any references on delete
			Panel* root = nullptr;
			Panel* active = nullptr;
			Panel* focus = nullptr;
			Panel* hover = nullptr;

			// Used to guard against recursive call bugs
			ENGINE_DEBUG_ONLY(bool focusGuard = false);
			ENGINE_DEBUG_ONLY(bool hoverGuard = false);

			std::vector<Panel*> hoverStack;
			std::vector<Panel*> hoverStackBack;
			bool hoverValid = false;

			std::vector<Panel*> focusStack; // Stored in reverse order: front() is the focused pane land back() is the root panel.
			std::vector<Panel*> focusStackBack;

			std::vector<Panel*> deferredDeleteQueue;
			std::vector<Panel*> deleteQueue;

			glm::vec2 view = {};
			glm::vec2 cursor = {};

			/* Callbacks */
			FlatHashMap<Panel*, MouseMoveCallback> mouseMoveCallbacks;
			FlatHashMap<Panel*, TextCallback> textCallbacks;
			FlatHashMap<Panel*, KeyCallback> keyCallbacks;

			int currPanelUpdateFunc = 0;
			PanelUpdateFuncId lastPanelUpdateId = {};
			struct PanelUpdateData { PanelUpdateFuncId id = {}; Panel* panel; PanelUpdateFunc func; };
			std::vector<PanelUpdateData> panelUpdateFunc;

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
			Clock::Duration autoscrollRate = clickRate / 5;

		public:
			Context(Gfx::ShaderLoader& shaderLoader, Gfx::TextureLoader& textureLoader, Camera& camera);
			Context(Context&) = delete;
			~Context();

			ENGINE_INLINE constexpr static auto getResizeBorderSize() noexcept { return resizeBorderSize; }
			ENGINE_INLINE float32 getScrollChars() const noexcept { return scrollChars; }
			ENGINE_INLINE float32 getScrollLines() const noexcept { return scrollLines; }
			ENGINE_INLINE Clock::Duration getAutoscrollSpeed() const noexcept { return autoscrollRate; }

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

			void unsetActive();
			void updateHover();

			template<class P, class... Args>
			P* constructPanel(Args&&... args) {
				auto* p = new P(this, std::forward<Args>(args)...);
				registerPanel(p);
				ENGINE_LOG2("constructPanel: {} = {}", (void*)p, typeid(P).name());
				return p;
			}

			template<class P, class... Args>
			P* createPanel(Panel* parent, Args&&... args) {
				ENGINE_DEBUG_ASSERT(parent != nullptr, "Creating a panel without a parent should be done with `constructPanel`.");
				auto* p = constructPanel<P>(std::forward<Args>(args)...);
				parent->addChild(p);
				return p;
			}

			/**
			 * Marks a panel for deletion.
			 * @ see deferredDeletePanels
			 */
			ENGINE_INLINE void deferredDeletePanel(Panel* panel){ return deferredDeletePanels(panel, panel); }

			/**
			 * Marks a set of siblings and their children for deletion.
			 * All panels marked are:
			 * - Removed from their parents.
			 * - Update and callbacks are removed.
			 * - State marked as deleted and disabled.
			 * - Added to list to be deleted at the end of frame.
			 */
			void deferredDeletePanels(Panel* first, Panel* last);
			
			ENGINE_INLINE void clearAllCallbacks(Panel* panel) {
				clearPanelUpdateFuncs(panel);
				deregisterMouseMove(panel);
				deregisterTextCallback(panel);
				deregisterPanel(panel);
			}

			ENGINE_INLINE PanelUpdateFuncId addPanelUpdateFunc(Panel* panel, PanelUpdateFunc&& func) {
				++lastPanelUpdateId.id;
				panelUpdateFunc.push_back({.id = lastPanelUpdateId, .panel = panel, .func = std::move(func)});
				ENGINE_WARN("CREATE: ", lastPanelUpdateId.id);
				return lastPanelUpdateId;
			}

			void removePanelUpdateFunc(PanelUpdateFuncId id) {
				ENGINE_DEBUG_ASSERT(id);
				const auto end = panelUpdateFunc.cend();
				for (auto it = panelUpdateFunc.cbegin(); it != end; ++it) {
					if (it->id == id) {
						ENGINE_LOG("ERASE: ", id.id);
						panelUpdateFunc.erase(it);
						return;
					}
				}
				ENGINE_WARN("Invalid panel update function id: ", id.id);
				ENGINE_DEBUG_BREAK;
			}

			void clearPanelUpdateFuncs(Panel* panel) {
				for (auto i=std::ssize(panelUpdateFunc)-1; i >= 0; --i) {
					if (panelUpdateFunc[i].panel == panel) {
						if (i <= currPanelUpdateFunc) { --currPanelUpdateFunc; }
						panelUpdateFunc.erase(panelUpdateFunc.begin() + i);
					}
				}
			}

			ENGINE_INLINE PanelUpdateFuncId createTimer(Panel* panel, Clock::Duration interval, PanelUpdateFunc&& func) {
				return addPanelUpdateFunc(nullptr, [interval, last=Clock::now(), func = std::move(func)](Panel* p) mutable {
					if (const auto now = Clock::now(); now - last >= interval) {
						func(p);
						last = now;
					}
				});
			}

			ENGINE_INLINE [[nodiscard]] auto createTimer(Clock::Duration interval, TimerUpdateFunc&& func) {
				return addPanelUpdateFunc(nullptr, [interval, last=Clock::now(), func = std::move(func)](Panel*) mutable {
					if (const auto now = Clock::now(); now - last >= interval) {
						func();
						last = now;
					}
				});
			}

			ENGINE_INLINE void deleteTimer(PanelUpdateFuncId id) {
				removePanelUpdateFunc(id);
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
			ENGINE_INLINE bool inFocusStack(const Panel* panel) const noexcept { return std::ranges::contains(focusStack, panel); }
			
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
			void setClipboard(ArrayView<const std::string_view> array) const;
			ENGINE_INLINE void setClipboard(const std::string_view view) const { setClipboard(ArrayView{view}); }

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
			bool onMouse(const Input::InputEvent event);

			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onMouseMove(const Input::InputEvent event);

			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onMouseWheel(const Input::InputEvent event);
			
			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onKey(const Input::InputEvent event);

			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onText(std::string_view str, Input::KeyCode code);

			void onResize(const int32 w, const int32 h);
			void onFocus(const bool has);

		private:
			/**
			 * Deletes all panels marked for deletion.
			 * @see deferredDeletePanels
			 */
			void deleteDeferredPanels();

			/**
			 * Calls delete on a set of siblings and their children.
			 * @see deleteDeferredPanels
			 * @see deferredDeletePanels
			 */
			void deleteSiblings(Panel* first, Panel* last);

			/**
			 * Walks a panel tree and performs the deferred deletion for all panels.
			 * @see deferredDeletePanels
			 */
			void cleanup(Panel* panel);

			ENGINE_INLINE void registerPanel(const Panel* panel) {
			}

			ENGINE_INLINE void deregisterPanel(const Panel* panel) {
				ENGINE_DEBUG_ASSERT(panel != nullptr, "Attempting to deregister nullptr.");
			}
	};
}
