// ImGui
#include <imgui.h>

// Engine
#include <Engine/Windows/OpenGLWindow.hpp>
#include <Engine/Input/InputState.hpp>


namespace Engine::ImGui {
	bool init(Engine::Windows::OpenGLWindow& hWnd);
	void shutdown();
	void newFrame();
	void draw();

	void mouseButtonCallback(const Engine::Input::InputState& is);
	void mouseMoveCallback(int x, int y);
	void scrollCallback(float xoffset, float yoffset);
	void keyCallback(const Engine::Input::InputState& is);
	void charCallback(unsigned int c);
}
