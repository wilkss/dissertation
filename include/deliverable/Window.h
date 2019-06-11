#pragma once

#include <string>

struct GLFWwindow;

namespace deliverable {

	class Window
	{
	public:
		Window(unsigned width, unsigned height, const std::string& title);
		~Window();

		void PollEvents();
		void SwapBuffers();
		bool ShouldClose() const;

		inline unsigned GetWidth() const { return m_Width; }
		inline unsigned GetHeight() const { return m_Height; }
		inline GLFWwindow* GetGLFWwindow() const { return m_Window; }

	private:
		unsigned m_Width;
		unsigned m_Height;
		GLFWwindow* m_Window;
	};

}