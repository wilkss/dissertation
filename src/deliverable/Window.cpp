#include "deliverable/Window.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace deliverable {

	Window::Window(unsigned width, unsigned height, const std::string& title)
		: m_Width(width), m_Height(height)
	{
		if (!glfwInit())
			throw std::runtime_error("Could not initialise GLFW");

		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		if (!m_Window)
			throw std::runtime_error("Could not create GLFW window");

		glfwMakeContextCurrent(m_Window);
		glfwSwapInterval(0);

		if (glewInit() != GLEW_OK)
			throw std::runtime_error("Could not initialise GLEW");
	}

	Window::~Window()
	{
		if (m_Window)
			glfwDestroyWindow(m_Window);

		glfwTerminate();
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	void Window::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_Window);
	}

}