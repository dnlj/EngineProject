#pragma once


namespace Engine::Windows {
	// TODO: name?
	class OpenGLWindow {
		public:
			OpenGLWindow();
			OpenGLWindow(const OpenGLWindow&) = delete;

			~OpenGLWindow();

			OpenGLWindow& operator=(const OpenGLWindow&) = delete;
			
		private:
			static void init();
			static LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		private:
			static constexpr wchar_t className[] = L"Engine_OpenGLWindow";
	};
}
