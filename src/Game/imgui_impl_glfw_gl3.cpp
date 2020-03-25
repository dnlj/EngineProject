// Implemented features:
//  [X] User texture binding. Cast 'GLuint' OpenGL texture identifier as void*/ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.
//  [X] Gamepad navigation mapping. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// ImGui
#include <imgui.h>

// STD // TODO: remove - unsued
#include <iostream>

// Engine
#include <Engine/Clock.hpp>

// Game
#include <Game/imgui_impl_glfw_gl3.hpp>


// GLFW data
namespace {
	using namespace Engine::Types;
	Engine::Windows::OpenGLWindow* g_Window;
	Engine::Clock::TimePoint g_Time;
	bool g_MouseJustPressed[3] = {false, false, false};
	struct {
		int lctrl;
		int rctrl;
		int lshift;
		int rshift;
		int lalt;
		int ralt;
		int lsuper;
		int rsuper;
	} g_KeyMap;

	// OpenGL3 data
	char         g_GlslVersion[32] = "#version 150";
	GLuint       g_FontTexture = 0;
	int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
	int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
	int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
	unsigned int g_VboHandle = 0, g_ElementsHandle = 0;
}

// OpenGL3 Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so. 
void ImGui_ImplGlfwGL3_RenderDrawData(ImDrawData* draw_data) {
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width == 0 || fb_height == 0) { return; }
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// Backup GL state
	GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
	glActiveTexture(GL_TEXTURE0);
	GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
	GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
	GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
	GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
	GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
	GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
	GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
	GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

	const float ortho_projection[4][4] = {
		{ 2.0f / io.DisplaySize.x, 0.0f,                     0.0f, 0.0f },
		{ 0.0f,                    2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
		{ 0.0f,                    0.0f,                    -1.0f, 0.0f },
		{-1.0f,                    1.0f,                     0.0f, 1.0f },
	};

	glUseProgram(g_ShaderHandle);
	glUniform1i(g_AttribLocationTex, 0);
	glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
	if (glBindSampler) glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.

	// Recreate the VAO every time 
	// (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them.)
	GLuint vao_handle = 0;
	glGenVertexArrays(1, &vao_handle);
	glBindVertexArray(vao_handle);
	glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
	glEnableVertexAttribArray(g_AttribLocationPosition);
	glEnableVertexAttribArray(g_AttribLocationUV);
	glEnableVertexAttribArray(g_AttribLocationColor);
	glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

	// Draw
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = 0;

		glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

			if (pcmd->UserCallback) {
				pcmd->UserCallback(cmd_list, pcmd);
			} else {
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
			}

			idx_buffer_offset += pcmd->ElemCount;
		}
	}

	glDeleteVertexArrays(1, &vao_handle);

	// Restore modified GL state
	glUseProgram(last_program);
	glBindTexture(GL_TEXTURE_2D, last_texture);
	if (glBindSampler) glBindSampler(0, last_sampler);
	glActiveTexture(last_active_texture);
	glBindVertexArray(last_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
	if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

// TODO:
//static const char* ImGui_ImplGlfwGL3_GetClipboardText(void* user_data) {
//	return 
//}

// TODO:
//static void ImGui_ImplGlfwGL3_SetClipboardText(void* user_data, const char* text) {
//	glfwSetClipboardString((GLFWwindow*)user_data, text);
//}

void ImGui_ImplGlfw_MouseButtonCallback(int button, bool action) {
	ImGui::GetIO().MouseDown[button] = action;
	g_MouseJustPressed[button] = g_MouseJustPressed[button] || action;
}

void ImGui_ImplGlfw_MouseMoveCallback(int x, int y) {
	ImGui::GetIO().MousePos = ImVec2{static_cast<float32>(x), static_cast<float32>(y)};
}

void ImGui_ImplGlfw_ScrollCallback(float xoffset, float yoffset) {
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += xoffset;
	io.MouseWheel += yoffset;
}

void ImGui_ImplGlfw_KeyCallback(int key, bool action) {
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[key] = action;
}

void ImGui_ImplGlfw_CharCallback(unsigned int c) {
	ImGuiIO& io = ImGui::GetIO();

	if (c > 0 && c < 0x10000) {
		io.AddInputCharacter((unsigned short)c);
	}
}

bool ImGui_ImplGlfwGL3_CreateFontsTexture() {
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

	// Upload texture to graphics system
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &g_FontTexture);
	glBindTexture(GL_TEXTURE_2D, g_FontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, last_texture);

	return true;
}

bool ImGui_ImplGlfwGL3_CreateDeviceObjects() {
	// Backup GL state
	GLint last_texture, last_array_buffer, last_vertex_array;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

	const GLchar* vertex_shader =
		"uniform mat4 ProjMtx;\n"
		"in vec2 Position;\n"
		"in vec2 UV;\n"
		"in vec4 Color;\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"	Frag_UV = UV;\n"
		"	Frag_Color = Color;\n"
		"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
		"}\n";

	const GLchar* fragment_shader =
		"uniform sampler2D Texture;\n"
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"out vec4 Out_Color;\n"
		"void main()\n"
		"{\n"
		"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
		"}\n";

	const GLchar* vertex_shader_with_version[2] = {g_GlslVersion, vertex_shader};
	const GLchar* fragment_shader_with_version[2] = {g_GlslVersion, fragment_shader};

	g_ShaderHandle = glCreateProgram();
	g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
	g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(g_VertHandle, 2, vertex_shader_with_version, NULL);
	glShaderSource(g_FragHandle, 2, fragment_shader_with_version, NULL);
	glCompileShader(g_VertHandle);
	glCompileShader(g_FragHandle);
	glAttachShader(g_ShaderHandle, g_VertHandle);
	glAttachShader(g_ShaderHandle, g_FragHandle);
	glLinkProgram(g_ShaderHandle);

	g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
	g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
	g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
	g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
	g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

	glGenBuffers(1, &g_VboHandle);
	glGenBuffers(1, &g_ElementsHandle);

	ImGui_ImplGlfwGL3_CreateFontsTexture();

	// Restore modified GL state
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBindVertexArray(last_vertex_array);

	return true;
}

void ImGui_ImplGlfwGL3_InvalidateDeviceObjects() {
	if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
	if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
	g_VboHandle = g_ElementsHandle = 0;

	if (g_ShaderHandle && g_VertHandle) glDetachShader(g_ShaderHandle, g_VertHandle);
	if (g_VertHandle) glDeleteShader(g_VertHandle);
	g_VertHandle = 0;

	if (g_ShaderHandle && g_FragHandle) glDetachShader(g_ShaderHandle, g_FragHandle);
	if (g_FragHandle) glDeleteShader(g_FragHandle);
	g_FragHandle = 0;

	if (g_ShaderHandle) glDeleteProgram(g_ShaderHandle);
	g_ShaderHandle = 0;

	if (g_FontTexture) {
		glDeleteTextures(1, &g_FontTexture);
		ImGui::GetIO().Fonts->TexID = 0;
		g_FontTexture = 0;
	}
}

bool ImGui_ImplGlfwGL3_Init(Engine::Windows::OpenGLWindow& window) {
	g_Window = &window;
	g_Time = Engine::Clock::now();

	// Store GLSL version string so we can refer to it later in case we recreate shaders. Note: GLSL version is NOT the same as GL version. Leave this to NULL if unsure.
	const char *glsl_version = "#version 150";

	IM_ASSERT((int)strlen(glsl_version) + 2 < IM_ARRAYSIZE(g_GlslVersion));
	strcpy(g_GlslVersion, glsl_version);
	strcat(g_GlslVersion, "\n");

	// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;   // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;    // We can honor io.WantSetMousePos requests (optional, rarely used)

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
	io.KeyMap[ImGuiKey_Tab] = MapVirtualKeyW(VK_TAB, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_LeftArrow] = MapVirtualKeyW(VK_LEFT, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_RightArrow] = MapVirtualKeyW(VK_RIGHT, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_UpArrow] = MapVirtualKeyW(VK_UP, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_DownArrow] = MapVirtualKeyW(VK_DOWN, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_PageUp] = MapVirtualKeyW(VK_PRIOR, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_PageDown] = MapVirtualKeyW(VK_NEXT, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_Home] = MapVirtualKeyW(VK_HOME, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_End] = MapVirtualKeyW(VK_END, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_Insert] = MapVirtualKeyW(VK_INSERT, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_Delete] = MapVirtualKeyW(VK_DELETE, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_Backspace] = MapVirtualKeyW(VK_BACK, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_Space] = MapVirtualKeyW(VK_SPACE, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_Enter] = MapVirtualKeyW(VK_RETURN, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_Escape] = MapVirtualKeyW(VK_ESCAPE, MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_A] = MapVirtualKeyW('A', MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_C] = MapVirtualKeyW('C', MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_V] = MapVirtualKeyW('V', MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_X] = MapVirtualKeyW('X', MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_Y] = MapVirtualKeyW('Y', MAPVK_VK_TO_VSC);
	io.KeyMap[ImGuiKey_Z] = MapVirtualKeyW('Z', MAPVK_VK_TO_VSC);

	g_KeyMap.lctrl = MapVirtualKeyW(VK_LCONTROL, MAPVK_VK_TO_VSC);
	g_KeyMap.rctrl = MapVirtualKeyW(VK_RCONTROL, MAPVK_VK_TO_VSC);
	g_KeyMap.lshift = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
	g_KeyMap.rshift = MapVirtualKeyW(VK_RSHIFT, MAPVK_VK_TO_VSC);
	g_KeyMap.lalt = MapVirtualKeyW(VK_LMENU, MAPVK_VK_TO_VSC);
	g_KeyMap.ralt = MapVirtualKeyW(VK_RMENU, MAPVK_VK_TO_VSC);
	g_KeyMap.lsuper = MapVirtualKeyW(VK_LWIN, MAPVK_VK_TO_VSC);
	g_KeyMap.rsuper = MapVirtualKeyW(VK_RWIN, MAPVK_VK_TO_VSC);

	//io.SetClipboardTextFn = ImGui_ImplGlfwGL3_SetClipboardText;
	//io.GetClipboardTextFn = ImGui_ImplGlfwGL3_GetClipboardText;
	//io.ClipboardUserData = g_Window;
	io.ImeWindowHandle = g_Window->getWin32WindowHandle();
	ImGui_ImplGlfwGL3_CreateDeviceObjects();
	return true;
}

void ImGui_ImplGlfwGL3_Shutdown() {
	ImGui_ImplGlfwGL3_InvalidateDeviceObjects();
}

void ImGui_ImplGlfwGL3_NewFrame() {
	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	const auto dispSize = g_Window->getFramebufferSize();
	const auto winSize = dispSize; // TODO: get correct value here to support high dpi

	// TODO: Engine::Glue
	io.DisplaySize = ImVec2(static_cast<float32>(winSize.x), static_cast<float32>(winSize.y));
	io.DisplayFramebufferScale = ImVec2(dispSize.x / static_cast<float32>(winSize.x), dispSize.y / static_cast<float32>(winSize.y));

	// Time step. Setup as system::tick or system::run? what is this used for? I think in run.
	const auto current_time = Engine::Clock::now();
	io.DeltaTime = Engine::Clock::Seconds{current_time - g_Time}.count();
	g_Time = current_time;

	// Modifier keys
	io.KeyCtrl = io.KeysDown[g_KeyMap.lctrl] || io.KeysDown[g_KeyMap.rctrl];
	io.KeyShift = io.KeysDown[g_KeyMap.lshift] || io.KeysDown[g_KeyMap.rshift];
	io.KeyAlt = io.KeysDown[g_KeyMap.lalt] || io.KeysDown[g_KeyMap.ralt];
	io.KeySuper = io.KeysDown[g_KeyMap.lsuper] || io.KeysDown[g_KeyMap.rsuper];

	// Update mouse buttons
	for (int i = 0; i < 3; i++) {
		io.MouseDown[i] = io.MouseDown[i] || g_MouseJustPressed[i];
		g_MouseJustPressed[i] = false;
	}

	{ // Update Cursors
		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
			// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
			::SetCursor(nullptr);
		} else {
			// Show OS mouse cursor
			LPTSTR win32_cursor = IDC_ARROW;
			switch (imgui_cursor) {
				case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
				case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
				case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
				case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
				case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
				case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
				case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
				case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
				case ImGuiMouseCursor_NotAllowed:   win32_cursor = IDC_NO; break;
			}
			::SetCursor(::LoadCursor(nullptr, win32_cursor));
		}
	}

	// TODO: Gamepad navigation. See imgui/examples.

	// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
	ImGui::NewFrame();
}
