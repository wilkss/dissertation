#include "deliverable/renderer/ImGuiRenderer.h"
#include "deliverable/Window.h"

// Uses imgui example code: https://github.com/ocornut/imgui/tree/master/examples

namespace deliverable {

	namespace {

		static const char* ImGui_ImplGlfw_GetClipboardText(void* user_data)
		{
			return glfwGetClipboardString((GLFWwindow*)user_data);
		}

		static void ImGui_ImplGlfw_SetClipboardText(void* user_data, const char* text)
		{
			glfwSetClipboardString((GLFWwindow*)user_data, text);
		}

		void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheelH += (float)xoffset;
			io.MouseWheel += (float)yoffset;
		}

		void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (action == GLFW_PRESS)
				io.KeysDown[key] = true;
			if (action == GLFW_RELEASE)
				io.KeysDown[key] = false;

			// Modifiers are not reliable across systems
			io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
			io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
			io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
			io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
		}

		void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (c > 0 && c < 0x10000)
				io.AddInputCharacter((unsigned short)c);
		}

	}

	ImGuiRenderer::ImGuiRenderer(Window* window)
	{
		g_Window = window->GetGLFWwindow();
		g_Time = 0.0;

		const char* glsl_version = "#version 430";
		strcpy(g_GlslVersionString, glsl_version);
		strcat(g_GlslVersionString, "\n");

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.BackendRendererName = "imgui_impl_opengl3";
		io.BackendPlatformName = "imgui_impl_glfw";

		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

#if defined(_WIN32)
		io.ImeWindowHandle = (void*)glfwGetWin32Window(g_Window);
#endif

		g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
		g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
		g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
		g_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

		glfwSetScrollCallback(g_Window, ImGui_ImplGlfw_ScrollCallback);
		glfwSetKeyCallback(g_Window, ImGui_ImplGlfw_KeyCallback);
		glfwSetCharCallback(g_Window, ImGui_ImplGlfw_CharCallback);
	}

	ImGuiRenderer::~ImGuiRenderer()
	{
		for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
		{
			glfwDestroyCursor(g_MouseCursors[cursor_n]);
			g_MouseCursors[cursor_n] = NULL;
		}

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

		if (g_FontTexture)
		{
			ImGuiIO& io = ImGui::GetIO();
			glDeleteTextures(1, &g_FontTexture);
			io.Fonts->TexID = 0;
			g_FontTexture = 0;
		}

		ImGui::DestroyContext();
	}

	bool ImGuiRenderer::CreateDeviceObjects()
	{
		// Backup GL state
		GLint last_texture, last_array_buffer, last_vertex_array;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

		const GLchar* vertex_shader_glsl_410_core =
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

		const GLchar* fragment_shader_glsl_410_core =
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"uniform sampler2D Texture;\n"
			"layout (location = 0) out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
			"}\n";

		// Select shaders matching our GLSL versions
		const GLchar* vertex_shader = vertex_shader_glsl_410_core;
		const GLchar* fragment_shader = fragment_shader_glsl_410_core;

		// Create shaders
		const GLchar* vertex_shader_with_version[2] = { g_GlslVersionString, vertex_shader };
		g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(g_VertHandle, 2, vertex_shader_with_version, NULL);
		glCompileShader(g_VertHandle);
		//CheckShader(g_VertHandle, "vertex shader");

		const GLchar* fragment_shader_with_version[2] = { g_GlslVersionString, fragment_shader };
		g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(g_FragHandle, 2, fragment_shader_with_version, NULL);
		glCompileShader(g_FragHandle);
		//CheckShader(g_FragHandle, "fragment shader");

		g_ShaderHandle = glCreateProgram();
		glAttachShader(g_ShaderHandle, g_VertHandle);
		glAttachShader(g_ShaderHandle, g_FragHandle);
		glLinkProgram(g_ShaderHandle);
		//CheckProgram(g_ShaderHandle, "shader program");

		g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
		g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
		g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
		g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
		g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

		// Create buffers
		glGenBuffers(1, &g_VboHandle);
		glGenBuffers(1, &g_ElementsHandle);

		CreateFontsTexture();

		// Restore modified GL state
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindVertexArray(last_vertex_array);

		return true;
	}

	bool ImGuiRenderer::CreateFontsTexture()
	{
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
		io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

		// Restore state
		glBindTexture(GL_TEXTURE_2D, last_texture);

		return true;
	}

	void ImGuiRenderer::NewFrame()
	{
		if (!g_FontTexture)
			CreateDeviceObjects();

		ImGuiIO& io = ImGui::GetIO();
		IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

		// Setup display size (every frame to accommodate for window resizing)
		int w, h;
		int display_w, display_h;
		glfwGetWindowSize(g_Window, &w, &h);
		glfwGetFramebufferSize(g_Window, &display_w, &display_h);
		io.DisplaySize = ImVec2((float)w, (float)h);
		if (w > 0 && h > 0)
			io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

		// Setup time step
		double current_time = glfwGetTime();
		io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
		g_Time = current_time;


		// Update buttons
		for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
			io.MouseDown[i] = g_MouseJustPressed[i] || glfwGetMouseButton(g_Window, i) != 0;

		// Update mouse position
		const ImVec2 mouse_pos_backup = io.MousePos;
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
		const bool focused = glfwGetWindowAttrib(g_Window, GLFW_FOCUSED) != 0;
		if (focused)
		{
			if (io.WantSetMousePos)
			{
				glfwSetCursorPos(g_Window, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
			}
			else
			{
				double mouse_x, mouse_y;
				glfwGetCursorPos(g_Window, &mouse_x, &mouse_y);
				io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
			}
		}

		// Update mouse cursor
		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
		{
			// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
			glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		else
		{
			// Show OS mouse cursor
			// FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
			glfwSetCursor(g_Window, g_MouseCursors[imgui_cursor] ? g_MouseCursors[imgui_cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
			glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		ImGui::NewFrame();
	}

	void ImGuiRenderer::Render()
	{
		ImGui::Render();

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		ImGuiIO& io = ImGui::GetIO();
		ImDrawData* draw_data = ImGui::GetDrawData();
		int fb_width = (int)(draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
		int fb_height = (int)(draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0)
			return;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		// Backup GL state
		GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
		glActiveTexture(GL_TEXTURE0);
		GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
		GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_SAMPLER_BINDING
		GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
		GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#ifdef GL_POLYGON_MODE
		GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
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
#ifdef GL_CLIP_ORIGIN
		GLenum last_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
		if (last_clip_origin == GL_UPPER_LEFT)
			clip_origin_lower_left = false;
#endif

		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

		// Setup viewport, orthographic projection matrix
		// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
		glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		const float ortho_projection[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
			{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
			{ 0.0f,         0.0f,        -1.0f,   0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
		};
		glUseProgram(g_ShaderHandle);
		glUniform1i(g_AttribLocationTex, 0);
		glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
#ifdef GL_SAMPLER_BINDING
		glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif
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
		ImVec2 pos = draw_data->DisplayPos;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			const ImDrawIdx* idx_buffer_offset = 0;

			glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					// User callback (registered via ImDrawList::AddCallback)
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					ImVec4 clip_rect = ImVec4(pcmd->ClipRect.x - pos.x, pcmd->ClipRect.y - pos.y, pcmd->ClipRect.z - pos.x, pcmd->ClipRect.w - pos.y);
					if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
					{
						// Apply scissor/clipping rectangle
						if (clip_origin_lower_left)
							glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
						else
							glScissor((int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z, (int)clip_rect.w); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)

						// Bind texture, Draw
						glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
						glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
					}
				}
				idx_buffer_offset += pcmd->ElemCount;
			}
		}
		glDeleteVertexArrays(1, &vao_handle);

		// Restore modified GL state
		glUseProgram(last_program);
		glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
		glBindSampler(0, last_sampler);
#endif
		glActiveTexture(last_active_texture);
		glBindVertexArray(last_vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
		glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
		if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
		glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
		glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
	}

}