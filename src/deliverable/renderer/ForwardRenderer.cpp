#include "deliverable/renderer/ForwardRenderer.h"
#include "deliverable/Window.h"

#include <GL/glew.h>

namespace deliverable {

	ForwardRenderer::ForwardRenderer(Window* window)
		: Renderer(window)
	{
		m_LightAccumulationShaderHandle = CreateShaderProgram("media/shaders/Object.vertex", "media/shaders/forward_pipeline/LightAccumulation.fragment");
		m_LightAccumulationShaderProjectionMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ProjectionMatrix");
		m_LightAccumulationShaderViewMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ViewMatrix");
		m_LightAccumulationShaderModelMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ModelMatrix");
	}

	ForwardRenderer::~ForwardRenderer()
	{
		glDeleteProgram(m_LightAccumulationShaderHandle);
	}

	void ForwardRenderer::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::vector<RenderCommand>& renderCommands, const std::vector<Light>& lights)
	{
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_LightAccumulationShaderHandle);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightsShaderStorageBufferHandle);
		glUniformMatrix4fv(m_LightAccumulationShaderProjectionMatrixHandle, 1, false, &projectionMatrix[0][0]);
		glUniformMatrix4fv(m_LightAccumulationShaderViewMatrixHandle, 1, false, &viewMatrix[0][0]);

		for (auto& renderCommand : renderCommands)
		{
			glUniformMatrix4fv(m_LightAccumulationShaderModelMatrixHandle, 1, false, &renderCommand.modelMatrix[0][0]);
			glBindVertexArray(renderCommand.mesh->vertexArrayHandle);
			glDrawElements(GL_TRIANGLES, renderCommand.mesh->indexCount, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	}

}