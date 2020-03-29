// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// ImGui
#include <imgui.h>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ImGui/ImGui.hpp>


// GLFW data
namespace {
	// TODO: rm globals
	using namespace Engine::Types;
	Engine::Window* g_Window;
	Engine::Clock::TimePoint g_Time;
	ImGuiMouseCursor g_LastCursor = ImGuiMouseCursor_COUNT;
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

	// OpenGL Data
	const char   g_GlslVersionString[] = "#version 330";
	GLuint       g_FontTexture = 0;
	GLuint       g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
	GLint        g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
	GLint        g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
	GLuint g_VboHandle = 0, g_ElementsHandle = 0;

	bool CheckShader(GLuint handle, const char* desc) {
		GLint status = 0, log_length = 0;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);

		if ((GLboolean)status == GL_FALSE) {
			fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s!\n", desc);
		}

		if (log_length > 1) {
			ImVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
		}

		return (GLboolean)status == GL_TRUE;
	}

	bool CheckProgram(GLuint handle, const char* desc) {
		GLint status = 0, log_length = 0;
		glGetProgramiv(handle, GL_LINK_STATUS, &status);
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE) {
			fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s! (with GLSL '%s')\n", desc, g_GlslVersionString);
		}

		if (log_length > 1) {
			ImVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
		}
		return (GLboolean)status == GL_TRUE;
	}

	void ImGui_ImplOpenGL3_SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, GLuint vertex_array_object) {
		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Setup viewport, orthographic projection matrix
		// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
		glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		const float ortho_projection[4][4] = {
			{ 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
			{ 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
			{ 0.0f,         0.0f,        -1.0f,   0.0f },
			{ (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
		};

		glUseProgram(g_ShaderHandle);
		glUniform1i(g_AttribLocationTex, 0);
		glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
		glBindSampler(0, 0);

		glBindVertexArray(vertex_array_object);

		// Bind vertex/index buffers and setup attributes for ImDrawVert
		glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
		glEnableVertexAttribArray(g_AttribLocationPosition);
		glEnableVertexAttribArray(g_AttribLocationUV);
		glEnableVertexAttribArray(g_AttribLocationColor);
		glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
		glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
	}

	void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data) {
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
		int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0)
			return;

		// Backup GL state
		GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
		glActiveTexture(GL_TEXTURE0);
		GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
		GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
		GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		GLint last_vertex_array_object; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array_object);
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
		bool clip_origin_lower_left = true;
		GLenum last_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
		if (last_clip_origin == GL_UPPER_LEFT) {
			clip_origin_lower_left = false;
		}

		// Setup desired GL state
		// Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
		// The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
		GLuint vertex_array_object = 0; // TODO: dont create a new vao every frame...
		glGenVertexArrays(1, &vertex_array_object);
		ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			// Upload vertex/index buffers
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback != NULL) {
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
					if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
						ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
					} else {
						pcmd->UserCallback(cmd_list, pcmd);
					}
				} else {
					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clip_rect;
					clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
					clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
					clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
					clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

					if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
						// Apply scissor/clipping rectangle
						if (clip_origin_lower_left) {
							glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
						} else {
							glScissor((int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z, (int)clip_rect.w); // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
						}

						// Bind texture, Draw
						glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
						glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)), (GLint)pcmd->VtxOffset);
						glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
					}
				}
			}
		}

		// Destroy the temporary VAO
		glDeleteVertexArrays(1, &vertex_array_object);

		// Restore modified GL state
		glUseProgram(last_program);
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindSampler(0, last_sampler);
		glActiveTexture(last_active_texture);
		glBindVertexArray(last_vertex_array_object);
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

	bool ImGui_ImplOpenGL3_CreateDeviceObjects() {
		// Backup GL state
		GLint last_texture, last_array_buffer;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		GLint last_vertex_array;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

		const GLchar* vertex_shader =
			"\n"
			"layout (location = 0) in vec2 Position;\n"
			"layout (location = 1) in vec2 UV;\n"
			"layout (location = 2) in vec4 Color;\n"
			"uniform mat4 ProjMtx;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"    Frag_UV = UV;\n"
			"    Frag_Color = Color;\n"
			"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* fragment_shader =
			"\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"uniform sampler2D Texture;\n"
			"layout (location = 0) out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
			"}\n";

		// Create shaders
		const GLchar* vertex_shader_with_version[2] = { g_GlslVersionString, vertex_shader };
		g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(g_VertHandle, 2, vertex_shader_with_version, NULL);
		glCompileShader(g_VertHandle);
		CheckShader(g_VertHandle, "vertex shader");

		const GLchar* fragment_shader_with_version[2] = { g_GlslVersionString, fragment_shader };
		g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(g_FragHandle, 2, fragment_shader_with_version, NULL);
		glCompileShader(g_FragHandle);
		CheckShader(g_FragHandle, "fragment shader");

		g_ShaderHandle = glCreateProgram();
		glAttachShader(g_ShaderHandle, g_VertHandle);
		glAttachShader(g_ShaderHandle, g_FragHandle);
		glLinkProgram(g_ShaderHandle);
		CheckProgram(g_ShaderHandle, "shader program");
		glDetachShader(g_ShaderHandle, g_VertHandle);
		glDetachShader(g_ShaderHandle, g_FragHandle);
		glDeleteShader(g_VertHandle);
		glDeleteShader(g_FragHandle);

		g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
		g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
		g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
		g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
		g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

		// Create buffers
		glGenBuffers(1, &g_VboHandle);
		glGenBuffers(1, &g_ElementsHandle);

		// Build texture atlas
		ImGuiIO& io = ::ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

		// Upload texture to graphics system
		glGenTextures(1, &g_FontTexture);
		glBindTexture(GL_TEXTURE_2D, g_FontTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

		// Restore modified GL state
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindVertexArray(last_vertex_array);

		return true;
	}

	void ImGui_ImplOpenGL3_DestroyDeviceObjects() {
		glDeleteBuffers(1, &g_VboHandle);
		g_VboHandle = 0;

		glDeleteBuffers(1, &g_ElementsHandle);
		g_ElementsHandle = 0;

		glDeleteProgram(g_ShaderHandle);
		g_ShaderHandle = 0;

		// Cleanup fonts
		ImGuiIO& io = ::ImGui::GetIO();
		glDeleteTextures(1, &g_FontTexture);
		io.Fonts->TexID = 0;
		g_FontTexture = 0;
	}

	bool ImGui_ImplOpenGL3_Init() {
		// Setup back-end capabilities flags
		ImGuiIO& io = ::ImGui::GetIO();
		io.BackendRendererName = "imgui_impl_opengl3";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		ImGui_ImplOpenGL3_CreateDeviceObjects();
		return true;
	}
}

namespace Engine::ImGui {
	// TODO:
	//static const char* ImGui_ImplGlfwGL3_GetClipboardText(void* user_data) {
	//	return 
	//}
	  
	// TODO:
	//static void ImGui_ImplGlfwGL3_SetClipboardText(void* user_data, const char* text) {
	//	glfwSetClipboardString((GLFWwindow*)user_data, text);
	//}

	void mouseButtonCallback(const Engine::Input::InputState& is) {
		const auto btn = is.id.code;
		::ImGui::GetIO().MouseDown[btn] = is.value;
		g_MouseJustPressed[btn] = g_MouseJustPressed[btn] || is.value;
	}

	void mouseMoveCallback(const Engine::Input::InputState& is) {
		::ImGui::GetIO().MousePos[is.id.code] = is.valuef;
	}

	void scrollCallback(float xoffset, float yoffset) {
		ImGuiIO& io = ::ImGui::GetIO();
		io.MouseWheelH += xoffset;
		io.MouseWheel += yoffset;
	}

	void keyCallback(const Engine::Input::InputState& is) {
		ImGuiIO& io = ::ImGui::GetIO();
		io.KeysDown[is.id.code & 0xFF] = is.value;
	}

	void charCallback(unsigned int c) {
		ImGuiIO& io = ::ImGui::GetIO();

		if (c > 0 && c < 0x10000) {
			io.AddInputCharacter((unsigned short)c);
		}
	}

	void mouseEnterCallback() {
		g_LastCursor = ImGuiMouseCursor_COUNT;
	}

	bool init(Engine::Window& window) {
		ImGui_ImplOpenGL3_Init();

		g_Window = &window;
		g_Time = Engine::Clock::now();

		// Setup back-end capabilities flags
		ImGuiIO& io = ::ImGui::GetIO();
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
		ImGui_ImplOpenGL3_CreateDeviceObjects();
		return true;
	}

	void shutdown() {
		ImGui_ImplOpenGL3_DestroyDeviceObjects();
	}

	void newFrame() {
		ImGuiIO& io = ::ImGui::GetIO();

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

		// Update Cursors
		ImGuiMouseCursor imgui_cursor = ::ImGui::GetMouseCursor();
		if (imgui_cursor != g_LastCursor) {
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
			g_LastCursor = imgui_cursor;
		}

		// TODO: Gamepad navigation. See imgui/examples.

		// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
		::ImGui::NewFrame();
	}

	void draw() {
		::ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());
	}
}
