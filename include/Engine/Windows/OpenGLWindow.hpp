#pragma once

// GL
#include <glloadgen/gl_core_4_5.hpp>
#include <wglext.h> // TODO: should be part of glloadgen or similar.

// Engine
#include <Engine/Windows/PixelFormat.hpp>
#include <Engine/Windows/ContextFormat.hpp>


namespace Engine::Windows {
	// TODO: vsync support
	// TODO: GLFW_OPENGL_DEBUG_CONTEXT
	// TODO: GLFW_SRGB_CAPABLE
	// TODO: name?
	class OpenGLWindow {
		public:
			using KeyPressCallback = void (*)(void* userdata, int scancode, bool extended);
			using KeyReleaseCallback = void (*)(void* userdata, int scancode, bool extended);
			using MousePressCallback = void (*)(void* userdata, int32 button);
			using MouseReleaseCallback = void (*)(void* userdata, int32 button);
			using MouseMoveCallback = void (*)(void* userdata, int32 x, int32 y);
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
			MousePressCallback mousePressCallback = nullptr;
			MouseReleaseCallback mouseReleaseCallback = nullptr;
			MouseMoveCallback mouseMoveCallback = nullptr;
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

			void setKeyPressCallback(KeyPressCallback callback);
			void setKeyReleaseCallback(KeyReleaseCallback callback);
			void setMousePressCallback(MousePressCallback callback);
			void setMouseReleaseCallback(MouseReleaseCallback callback);
			void setMouseMoveCallback(MouseMoveCallback callback);
			void setResizeCallback(ResizeCallback callback);

		private:
			static WGLPointers init();
			static LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	};
}
