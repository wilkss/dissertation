#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

namespace deliverable {

	struct Mesh
	{
		unsigned vertexArrayHandle;
		unsigned indexCount;
	};

	struct Light
	{
		glm::vec4 positionAndRadius;
		glm::vec4 color;
	};

	struct RenderCommand
	{
		glm::mat4 modelMatrix;
		Mesh* mesh;
	};

	class Window;

	class Renderer
	{
	public:
		Renderer(Window* window);
		virtual ~Renderer();

		virtual void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::vector<RenderCommand>& renderCommands, const std::vector<Light>& lights) = 0;

		void UpdateLightsShaderStorageBuffer(const std::vector<Light>& lights);

		unsigned CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);
		unsigned CreateShaderProgram(const std::string& computeShaderFilePath);
		unsigned CompileShader(const std::string& filepath, unsigned type);

	protected:
		Window* m_Window;
		unsigned m_ScreenSpaceQuadVertexArrayHandle;
		unsigned m_LightsShaderStorageBufferHandle;
	};

}