#include "deliverable/renderer/Renderer.h"

#include <GL/glew.h>

#include <iostream>
#include <fstream>

namespace deliverable {

	Renderer::Renderer(Window* window)
		: m_Window(window)
	{
		glGenBuffers(1, &m_LightsShaderStorageBufferHandle);
	}

	Renderer::~Renderer()
	{
		glDeleteBuffers(1, &m_LightsShaderStorageBufferHandle);
	}

	void Renderer::UpdateLightsShaderStorageBuffer(const std::vector<Light>& lights)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightsShaderStorageBufferHandle);
		if (lights.empty())
			glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
		else
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Light) * lights.size(), &lights[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	unsigned Renderer::CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
	{
		unsigned programHandle = glCreateProgram();

		unsigned vertexShaderHandle = CompileShader(vertexShaderFilePath, GL_VERTEX_SHADER);
		unsigned fragmentShaderHandle = CompileShader(fragmentShaderFilePath, GL_FRAGMENT_SHADER);

		glAttachShader(programHandle, vertexShaderHandle);
		glAttachShader(programHandle, fragmentShaderHandle);

		glLinkProgram(programHandle);

		int result;
		glGetProgramiv(programHandle, GL_LINK_STATUS, &result);
		if (result == GL_FALSE)
		{
			int length;
			glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &length);
			char* message = new char[length];
			glGetProgramInfoLog(programHandle, length, &length, message);
			std::cout << "Failed to link shader program" << std::endl;
			std::cout << message << std::endl;
			delete[] message;
		}

		glDeleteShader(vertexShaderHandle);
		glDeleteShader(fragmentShaderHandle);
		return programHandle;
	}

	unsigned Renderer::CreateShaderProgram(const std::string& computeShaderFilePath)
	{
		unsigned programHandle = glCreateProgram();

		unsigned computeShaderHandle = CompileShader(computeShaderFilePath, GL_COMPUTE_SHADER);

		glAttachShader(programHandle, computeShaderHandle);

		glLinkProgram(programHandle);

		int result;
		glGetProgramiv(programHandle, GL_LINK_STATUS, &result);
		if (result == GL_FALSE)
		{
			int length;
			glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &length);
			char* message = new char[length];
			glGetProgramInfoLog(programHandle, length, &length, message);
			std::cout << "Failed to link shader program" << std::endl;
			std::cout << message << std::endl;
			delete[] message;
		}

		glDeleteShader(computeShaderHandle);
		return programHandle;
	}

	unsigned Renderer::CompileShader(const std::string& filepath, unsigned type)
	{
		std::ifstream file(filepath);
		if (!file.is_open())
		{
			std::cout << "Failed to open shader: " << filepath << std::endl;
			return 0;
		}

		std::string source;
		std::string line;
		while (std::getline(file, line))
			source += line + '\n';

		unsigned shaderHandle = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(shaderHandle, 1, &src, nullptr);
		glCompileShader(shaderHandle);

		int result;
		glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
		if (result == GL_FALSE)
		{
			int length;
			glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
			char* message = new char[length];
			glGetShaderInfoLog(shaderHandle, length, &length, message);
			glDeleteShader(shaderHandle);
			std::cout << "Failed to compile shader: " << filepath << std::endl;
			std::cout << message << std::endl;
			delete[] message;
			return 0;
		}

		return shaderHandle;
	}

}