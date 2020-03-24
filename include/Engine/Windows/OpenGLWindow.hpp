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

		private:
			static WGLPointers init();
			static LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	};


	// TODO: do we reall want press/release? or just one?
	// TODO: nope. none of this.
	inline void* userdata = nullptr;
	inline void (*keyPressCallback)(int scancode, bool extended) = nullptr;
	inline void (*keyReleaseCallback)(int scancode, bool extended) = nullptr;
	inline void (*mousePressCallback)(int32 button) = nullptr;
	inline void (*mouseReleaseCallback)(int32 button) = nullptr;
	inline void (*mouseMoveCallback)(int32 x, int32 y) = nullptr;
	inline void (*sizeCallback)(int32 w, int32 h) = nullptr;
}
