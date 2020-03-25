// ImGui
#include <imgui.h>

// Engine
#include <Engine/Windows/OpenGLWindow.hpp>


namespace Engine::ImGui {
	bool init(Engine::Windows::OpenGLWindow& hWnd);
	void shutdown();
	void newFrame();
	void renderDrawData(ImDrawData* draw_data);

	void invalidateDeviceObjects();
	bool createDeviceObjects();

	void mouseButtonCallback(int button, bool action);
	void mouseMoveCallback(int x, int y);
	void scrollCallback(float xoffset, float yoffset);
	void keyCallback(int button, bool action);
	void charCallback(unsigned int c);
}
