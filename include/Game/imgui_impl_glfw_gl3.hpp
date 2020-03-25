// ImGui
#include <imgui.h>

// Engine
#include <Engine/Windows/OpenGLWindow.hpp>


bool ImGui_ImplGlfwGL3_Init(Engine::Windows::OpenGLWindow& hWnd);
void ImGui_ImplGlfwGL3_Shutdown();
void ImGui_ImplGlfwGL3_NewFrame();
void ImGui_ImplGlfwGL3_RenderDrawData(ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing ImGui state.
void ImGui_ImplGlfwGL3_InvalidateDeviceObjects();
bool ImGui_ImplGlfwGL3_CreateDeviceObjects();

// GLFW callbacks (installed by default if you enable 'install_callbacks' during initialization)
// Provided here if you want to chain callbacks.
// You can also handle inputs yourself and use those as a reference.
void ImGui_ImplGlfw_MouseButtonCallback(int button, bool action);
void ImGui_ImplGlfw_MouseMoveCallback(int x, int y);
void ImGui_ImplGlfw_ScrollCallback(float xoffset, float yoffset);
void ImGui_ImplGlfw_KeyCallback(int button, bool action);
void ImGui_ImplGlfw_CharCallback(unsigned int c);
