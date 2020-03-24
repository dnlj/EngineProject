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

			HWND window = nullptr;
			HDC deviceContext = nullptr;
			HGLRC renderContext = nullptr;

		public:
			OpenGLWindow(const PixelFormat& pixelFormat, const ContextFormat& contextFormat);
			OpenGLWindow(const OpenGLWindow&) = delete;

			~OpenGLWindow(); // TODO: cleanup Windows objects

			OpenGLWindow& operator=(const OpenGLWindow&) = delete;

			void show();

			void makeContextCurrent();

			void poll();

			void swapBuffers();

			

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
	inline void (*sizingCallback)(int32 x, int32 y, int32 w, int32 h) = nullptr;
}
