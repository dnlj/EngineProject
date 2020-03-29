#pragma once

// GL
#include <glloadgen/gl_core_4_5.hpp>
#include <wglext.h> // TODO: should be part of glloadgen or similar.

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Win32/PixelFormat.hpp>
#include <Engine/Win32/ContextFormat.hpp>
#include <Engine/Input/InputEvent.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/InputEvent.hpp>


namespace Engine::Win32 {
	// TODO: vsync support
	// TODO: GLFW_OPENGL_DEBUG_CONTEXT
	// TODO: GLFW_SRGB_CAPABLE
	// TODO: name?
	// TODO: High DPI https://docs.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
	// TODO: mouse buttons other than l/r
	class OpenGLWindow {
		public:
			using ResizeCallback = void (*)(void* userdata, int32 w, int32 h);
			using KeyCallback = void (*)(void* userdata, Input::InputEvent event);
			//using KeyReleaseCallback = void (*)(void* userdata, uint16 scancode, bool extended);
			using CharCallback = void (*)(void* userdata, wchar_t character);
			using MouseButtonCallback = void (*)(void* userdata, Input::InputEvent event);
			//using MouseReleaseCallback = void (*)(void* userdata, Input::InputEvent event);
			using MouseMoveCallback = void (*)(void* userdata, Input::InputEvent event);
			using MouseWheelCallback = void (*)(void* userdata, float32 x, float32 y);
			using MouseLeaveCallback = void (*)(void* userdata);
			using MouseEnterCallback = void (*)(void* userdata);
			void* userdata; // TODO: rm - figure out better

			// TODO: just make virtual member functions?
			ResizeCallback resizeCallback = nullptr;
			KeyCallback keyCallback = nullptr;
			//KeyReleaseCallback keyReleaseCallback = nullptr;
			CharCallback charCallback = nullptr;
			MouseButtonCallback mouseButtonCallback = nullptr;
			//MouseReleaseCallback mouseReleaseCallback = nullptr;
			MouseMoveCallback mouseMoveCallback = nullptr;
			MouseWheelCallback mouseWheelCallback = nullptr;
			MouseLeaveCallback mouseLeaveCallback = nullptr;
			MouseEnterCallback mouseEnterCallback = nullptr;

		private:
			struct WGLPointers {
				PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
				PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
			};

			struct KeyBoardState {
				// We use 0xAA as a custom prefix to denote that the stored code is actually a virtual key (Win32 VK_* enum).
				// This is needed because some keys don't produce a scancode (RAWKEYBOARD::MakeCode)
				// but do produce a virtual key (RAWKEYBOARD::VKey). One example of this is WASD Code V2B media keys.
				// We could map the virtual key back to a scancode using MapVirtualKeyW
				// but not all virtual keys have corresponding scancodes so instead we store the VK_* value with
				// the prefix 0xAA = 0b10101010 which is mostly arbitrary value.

				// Must be stored so we can detect repeat (held) keys
				bool state[0xFF * 4]; // For each prefix: 0x00, 0xE0, 0xE1, 0xAA (custom)
			};

			static constexpr wchar_t className[] = L"Engine_Windows_OpenGLWindow";

			HWND windowHandle = nullptr;
			HDC deviceContext = nullptr;
			HGLRC renderContext = nullptr;
			bool close = false;
			bool mouseInWindow = false;
			glm::ivec2 lastMousePos = {0, 0};
			BYTE rawInputBuffer[128];
			// TODO: if we want to save key bindings to a config file we will need a way to back to HANDLE and use RIDI_DEVICENAME to save
			FlatHashMap<HANDLE, uint8> keyboardHandleToIndex;
			std::vector<KeyBoardState> keyboardData;

		public:
			OpenGLWindow(const PixelFormat& pixelFormat, const ContextFormat& contextFormat);
			OpenGLWindow(const OpenGLWindow&) = delete;

			~OpenGLWindow(); // TODO: cleanup Windows objects

			OpenGLWindow& operator=(const OpenGLWindow&) = delete;

			void show();

			void makeContextCurrent();

			void poll();

			void swapBuffers();

			HWND getWin32WindowHandle() const;

			bool shouldClose() const;

			glm::ivec2 getFramebufferSize() const;

			// TODO: std::string getClipboardText() const;
			// TODO: void setClipboardText

		private:
			static WGLPointers init();

			static LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			template<UINT Msg>
			static LRESULT processMessage(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) = delete;
	};
}
