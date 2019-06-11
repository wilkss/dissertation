#include "deliverable/renderer/ForwardPlusRenderer.h"
#include "deliverable/Window.h"

#include <GL/glew.h>

namespace deliverable {

	ForwardPlusRenderer::ForwardPlusRenderer(Window* window)
		: TiledRenderer(window)
	{
		glGenTextures(1, &m_DepthPrepassDepthTexture);
		glBindTexture(GL_TEXTURE_2D, m_DepthPrepassDepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_Window->GetWidth(), m_Window->GetHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &m_DepthPrepassFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthPrepassFbo);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthPrepassDepthTexture, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		m_EmptyShaderHandle = CreateShaderProgram("media/shaders/Object.vertex", "media/shaders/Empty.fragment");
		m_EmptyShaderProjectionMatrixHandle = glGetUniformLocation(m_EmptyShaderHandle, "u_ProjectionMatrix");
		m_EmptyShaderViewMatrixHandle = glGetUniformLocation(m_EmptyShaderHandle, "u_ViewMatrix");
		m_EmptyShaderModelMatrixHandle = glGetUniformLocation(m_EmptyShaderHandle, "u_ModelMatrix");

		m_LightCullingShaderHandle = CreateShaderProgram("media/shaders/tiled_forward_pipeline/LightCulling.compute");
		m_LightCullingShaderDepthTextureHandle = glGetUniformLocation(m_LightCullingShaderHandle, "u_DepthTexture");
		m_LightCullingShaderProjectionMatrixHandle = glGetUniformLocation(m_LightCullingShaderHandle, "u_ProjectionMatrix");
		m_LightCullingShaderViewMatrixHandle = glGetUniformLocation(m_LightCullingShaderHandle, "u_ViewMatrix");
		m_LightCullingShaderViewProjectionMatrixHandle = glGetUniformLocation(m_LightCullingShaderHandle, "u_ViewProjectionMatrix");
		m_LightCullingShaderInverseProjectionMatrixHandle = glGetUniformLocation(m_LightCullingShaderHandle, "u_InverseProjectionMatrix");
		m_LightCullingShaderScreenSizeHandle = glGetUniformLocation(m_LightCullingShaderHandle, "u_ScreenSize");

		m_LightAccumulationShaderHandle = CreateShaderProgram("media/shaders/Object.vertex", "media/shaders/tiled_forward_pipeline/LightAccumulation.fragment");
		m_LightAccumulationShaderProjectionMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ProjectionMatrix");
		m_LightAccumulationShaderViewMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ViewMatrix");
		m_LightAccumulationShaderModelMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ModelMatrix");
		m_LightAccumulationShaderNumberOfTilesXHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_NumberOfTilesX");

		glGenBuffers(1, &m_VisibleIndicesShaderStorageBufferHandle);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_VisibleIndicesShaderStorageBufferHandle);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_WorkGroupsX * m_WorkGroupsY * sizeof(unsigned int) * MAX_LIGHTS_PER_TILE, nullptr, GL_STATIC_DRAW);
	}

	ForwardPlusRenderer::~ForwardPlusRenderer()
	{
		glDeleteBuffers(1, &m_VisibleIndicesShaderStorageBufferHandle);

		glDeleteFramebuffers(1, &m_DepthPrepassFbo);
		glDeleteTextures(1, &m_DepthPrepassDepthTexture);

		glDeleteProgram(m_LightCullingShaderHandle);
		glDeleteProgram(m_LightAccumulationShaderHandle);
		glDeleteProgram(m_EmptyShaderHandle);
	}

	void ForwardPlusRenderer::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::vector<RenderCommand>& renderCommands, const std::vector<Light>& lights)
	{
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthPrepassFbo);
		{
			glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
			glClear(GL_DEPTH_BUFFER_BIT);

			glUseProgram(m_EmptyShaderHandle);
			glUniformMatrix4fv(m_EmptyShaderProjectionMatrixHandle, 1, false, &projectionMatrix[0][0]);
			glUniformMatrix4fv(m_EmptyShaderViewMatrixHandle, 1, false, &viewMatrix[0][0]);

			for (auto& renderCommand : renderCommands)
			{
				glUniformMatrix4fv(m_EmptyShaderModelMatrixHandle, 1, false, &renderCommand.modelMatrix[0][0]);
				glBindVertexArray(renderCommand.mesh->vertexArrayHandle);
				glDrawElements(GL_TRIANGLES, renderCommand.mesh->indexCount, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_LightCullingShaderHandle);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(m_LightCullingShaderDepthTextureHandle, 0);
		glBindTexture(GL_TEXTURE_2D, m_DepthPrepassDepthTexture);
		glUniformMatrix4fv(m_LightCullingShaderProjectionMatrixHandle, 1, false, &projectionMatrix[0][0]);
		glUniformMatrix4fv(m_LightCullingShaderViewMatrixHandle, 1, false, &viewMatrix[0][0]);
		glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
		glUniformMatrix4fv(m_LightCullingShaderViewProjectionMatrixHandle, 1, false, &viewProjectionMatrix[0][0]);
		glm::mat4 inverseProjectionMatrix = glm::inverse(projectionMatrix);
		glUniformMatrix4fv(m_LightCullingShaderInverseProjectionMatrixHandle, 1, false, &inverseProjectionMatrix[0][0]);
		glUniform2i(m_LightCullingShaderScreenSizeHandle, m_Window->GetWidth(), m_Window->GetHeight());

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_LightsShaderStorageBufferHandle);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_VisibleIndicesShaderStorageBufferHandle);
		glDispatchCompute(m_WorkGroupsX, m_WorkGroupsY, 1);

		glUseProgram(m_LightAccumulationShaderHandle);
		glUniform1i(m_LightAccumulationShaderNumberOfTilesXHandle, m_WorkGroupsX);
		glUniformMatrix4fv(m_LightAccumulationShaderProjectionMatrixHandle, 1, false, &projectionMatrix[0][0]);
		glUniformMatrix4fv(m_LightAccumulationShaderViewMatrixHandle, 1, false, &viewMatrix[0][0]);

		for (auto& renderCommand : renderCommands)
		{
			glUniformMatrix4fv(m_LightAccumulationShaderModelMatrixHandle, 1, false, &renderCommand.modelMatrix[0][0]);
			glBindVertexArray(renderCommand.mesh->vertexArrayHandle);
			glDrawElements(GL_TRIANGLES, renderCommand.mesh->indexCount, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	}

}