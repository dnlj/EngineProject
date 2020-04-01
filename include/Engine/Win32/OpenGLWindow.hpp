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
#include <Engine/WindowCallbacks.hpp>


namespace Engine::Win32 {
	// TODO: vsync support
	// TODO: GLFW_OPENGL_DEBUG_CONTEXT
	// TODO: GLFW_SRGB_CAPABLE
	// TODO: name?
	// TODO: High DPI https://docs.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
	// TODO: mouse buttons other than l/r
	class OpenGLWindow {
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

			static constexpr wchar_t className[] = L"Engine::Win32::OpenGLWindow";
			
			// TODO: make template after de-globalizing UI/ImGui
			WindowCallbackFunctions& callbacks;
			HWND windowHandle = nullptr;
			HDC deviceContext = nullptr;
			HGLRC renderContext = nullptr;
			bool close = false;
			bool mouseInWindow = false;
			glm::ivec2 lastMousePos = {0, 0};
			BYTE rawInputBuffer[128];

			// TODO: if we want to save key bindings to a config file we will need a way to back to HANDLE and use RIDI_DEVICENAME to save
			std::vector<HANDLE> keyboardHandleToIndex;
			std::vector<KeyBoardState> keyboardData;

		public:
			OpenGLWindow(const PixelFormat& pixelFormat, const ContextFormat& contextFormat, WindowCallbackFunctions& callbacks);
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

			uint8 getKeyboardId(HANDLE handle);

		private:
			static WGLPointers init();

			static LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			template<UINT Msg>
			static LRESULT processMessage(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) = delete;
	};
}
