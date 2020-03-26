#pragma once

// GL
#include <glloadgen/gl_core_4_5.hpp>
#include <wglext.h> // TODO: should be part of glloadgen or similar.

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Win32/PixelFormat.hpp>
#include <Engine/Win32/ContextFormat.hpp>



namespace Engine::Win32 {
	// TODO: vsync support
	// TODO: GLFW_OPENGL_DEBUG_CONTEXT
	// TODO: GLFW_SRGB_CAPABLE
	// TODO: name?
	// TODO: High DPI https://docs.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
	class OpenGLWindow {
		public:
			using KeyPressCallback = void (*)(void* userdata, int scancode, bool extended);
			using KeyReleaseCallback = void (*)(void* userdata, int scancode, bool extended);
			using CharCallback = void (*)(void* userdata, wchar_t character);
			using MousePressCallback = void (*)(void* userdata, int32 button);
			using MouseReleaseCallback = void (*)(void* userdata, int32 button);
			using MouseMoveCallback = void (*)(void* userdata, int32 x, int32 y);
			using MouseWheelCallback = void (*)(void* userdata, float32 x, float32 y);
			using ResizeCallback = void (*)(void* userdata, int32 w, int32 h);
			void* userdata; // TODO: rm - figure out better

		private:
			struct WGLPointers {
				PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
				PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
			};

			static constexpr wchar_t className[] = L"Engine_Windows_OpenGLWindow";

			HWND windowHandle = nullptr;
			HDC deviceContext = nullptr;
			HGLRC renderContext = nullptr;
			bool close = false;

			KeyPressCallback keyPressCallback = nullptr;
			KeyReleaseCallback keyReleaseCallback = nullptr;
			CharCallback charCallback = nullptr;
			MousePressCallback mousePressCallback = nullptr;
			MouseReleaseCallback mouseReleaseCallback = nullptr;
			MouseMoveCallback mouseMoveCallback = nullptr;
			MouseWheelCallback mouseWheelCallback = nullptr;
			ResizeCallback resizeCallback = nullptr;

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
			// TODO: void setClipboardText(const 

			// TODO: make these all "setOnKeyPressCallback"
			void setKeyPressCallback(KeyPressCallback callback);
			void setKeyReleaseCallback(KeyReleaseCallback callback);
			void setCharCallback(CharCallback callback);
			void setMousePressCallback(MousePressCallback callback);
			void setMouseReleaseCallback(MouseReleaseCallback callback);
			void setMouseMoveCallback(MouseMoveCallback callback);
			void setMouseWheelCallback(MouseWheelCallback callback);
			void setResizeCallback(ResizeCallback callback);

		private:
			static WGLPointers init();
			static LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	};
}
