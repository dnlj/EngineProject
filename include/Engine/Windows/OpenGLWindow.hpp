#pragma once

// GL
#include <glloadgen/gl_core_4_5.hpp>
#include <wglext.h> // TODO: should be part of glloadgen or similar.


namespace Engine::Windows {
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
			OpenGLWindow();
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
}
