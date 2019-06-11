#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include <imgui/imgui.h>

// Uses imgui example code: https://github.com/ocornut/imgui/tree/master/examples

namespace deliverable {

	class Window;

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer(Window* window);
		~ImGuiRenderer();

		bool CreateDeviceObjects();
		bool CreateFontsTexture();

		void NewFrame();
		void Render();

	private:
		GLFWwindow*          g_Window = NULL;
		double g_Time;
		bool                 g_MouseJustPressed[5] = { false, false, false, false, false };
		GLuint       g_FontTexture = 0;
		GLuint       g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
		int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
		int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
		unsigned int g_VboHandle = 0, g_ElementsHandle = 0;
		char         g_GlslVersionString[32] = "";
		GLFWcursor*          g_MouseCursors[ImGuiMouseCursor_COUNT] = { 0 };
	};

}