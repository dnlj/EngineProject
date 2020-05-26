// ImGui
#define IMGUI_USER_CONFIG <Engine/ImGui/Config.hpp>
#include <imgui.h>

// ImPlot
#include <implot.h>

// Engine
#include <Engine/ImGui/Operators.hpp>
#include <Engine/Window.hpp>
#include <Engine/Input/InputState.hpp>


namespace Engine::ImGui {
	bool init(Engine::Window& hWnd);
	void shutdown();
	void newFrame();
	void draw();

	void mouseButtonCallback(const Engine::Input::InputState& is);
	void mouseMoveCallback(const Engine::Input::InputState& is);
	void scrollCallback(float xoffset, float yoffset);
	void keyCallback(const Engine::Input::InputState& is);
	void charCallback(unsigned int c);
	void mouseEnterCallback();
}
