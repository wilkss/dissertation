#include "deliverable/renderer/TiledDeferredRenderer.h"
#include "deliverable/Window.h"

#include <GL/glew.h>

namespace deliverable {

	TiledDeferredRenderer::TiledDeferredRenderer(Window* window, unsigned screenSpaceQuadVertexArrayHandle)
		: TiledRenderer(window), m_ScreenSpaceQuadVertexArrayHandle(screenSpaceQuadVertexArrayHandle)
	{
		m_GeometryShaderHandle = CreateShaderProgram("media/shaders/Object.vertex", "media/shaders/deferred_pipeline/GBuffer.fragment");
		m_GeometryShaderProjectionMatrixHandle = glGetUniformLocation(m_GeometryShaderHandle, "u_ProjectionMatrix");
		m_GeometryShaderViewMatrixHandle = glGetUniformLocation(m_GeometryShaderHandle, "u_ViewMatrix");
		m_GeometryShaderModelMatrixHandle = glGetUniformLocation(m_GeometryShaderHandle, "u_ModelMatrix");

		m_LightAccumulationShaderHandle = CreateShaderProgram("media/shaders/tiled_deferred_pipeline/LightAccumulation.compute");
		m_LightAccumulationShaderGBufferTexture0Handle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_GBufferTexture0");
		m_LightAccumulationShaderGBufferTexture1Handle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_GBufferTexture1");
		m_LightAccumulationShaderGBufferTexture2Handle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_GBufferTexture2");
		m_LightAccumulationShaderDepthTextureHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_DepthTexture");
		m_LightAccumulationShaderProjectionMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ProjectionMatrix");
		m_LightAccumulationShaderViewMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ViewMatrix");
		m_LightAccumulationShaderViewProjectionMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ViewProjectionMatrix");
		m_LightAccumulationShaderInverseProjectionMatrixHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_InverseProjectionMatrix");
		m_LightAccumulationShaderScreenSizeHandle = glGetUniformLocation(m_LightAccumulationShaderHandle, "u_ScreenSize");

		m_CompositeShaderHandle = CreateShaderProgram("media/shaders/ScreenSpaceQuad.vertex", "media/shaders/tiled_deferred_pipeline/Composite.fragment");
		m_CompositeShaderGBufferTexture0Handle = glGetUniformLocation(m_CompositeShaderHandle, "u_GBufferTexture0");
		m_CompositeShaderGBufferTexture1Handle = glGetUniformLocation(m_CompositeShaderHandle, "u_GBufferTexture1");
		m_CompositeShaderGBufferTexture2Handle = glGetUniformLocation(m_CompositeShaderHandle, "u_GBufferTexture2");
		m_CompositeShaderGBufferTexture3Handle = glGetUniformLocation(m_CompositeShaderHandle, "u_GBufferTexture3");
		m_CompositeShaderDepthTextureHandle = glGetUniformLocation(m_CompositeShaderHandle, "u_DepthTexture");

		glGenTextures(1, &m_GeometryDiffuseTextureHandle);
		glBindTexture(GL_TEXTURE_2D, m_GeometryDiffuseTextureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Window->GetWidth(), m_Window->GetHeight(), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &m_GeometryNormalsTextureHandle);
		glBindTexture(GL_TEXTURE_2D, m_GeometryNormalsTextureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Window->GetWidth(), m_Window->GetHeight(), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &m_GeometryPositionTextureHandle);
		glBindTexture(GL_TEXTURE_2D, m_GeometryPositionTextureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Window->GetWidth(), m_Window->GetHeight(), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &m_GeometryLightAccumulationTextureHandle);
		glBindTexture(GL_TEXTURE_2D, m_GeometryLightAccumulationTextureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Window->GetWidth(), m_Window->GetHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &m_GeometryDepthTextureHandle);
		glBindTexture(GL_TEXTURE_2D, m_GeometryDepthTextureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_Window->GetWidth(), m_Window->GetHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &m_GeometryBufferHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryBufferHandle);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GeometryDiffuseTextureHandle, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GeometryNormalsTextureHandle, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GeometryPositionTextureHandle, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_GeometryLightAccumulationTextureHandle, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_GeometryDepthTextureHandle, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	TiledDeferredRenderer::~TiledDeferredRenderer()
	{
		glDeleteFramebuffers(1, &m_GeometryBufferHandle);
		glDeleteTextures(1, &m_GeometryDiffuseTextureHandle);
		glDeleteTextures(1, &m_GeometryNormalsTextureHandle);
		glDeleteTextures(1, &m_GeometryPositionTextureHandle);
		glDeleteTextures(1, &m_GeometryLightAccumulationTextureHandle);
		glDeleteTextures(1, &m_GeometryDepthTextureHandle);

		glDeleteProgram(m_GeometryShaderHandle);
		glDeleteProgram(m_LightAccumulationShaderHandle);
		glDeleteProgram(m_CompositeShaderHandle);
	}

	void TiledDeferredRenderer::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::vector<RenderCommand>& renderCommands, const std::vector<Light>& lights)
	{
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryBufferHandle);
		{
			glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			glDrawBuffers(3, drawBuffers);

			glUseProgram(m_GeometryShaderHandle);
			glUniformMatrix4fv(m_GeometryShaderProjectionMatrixHandle, 1, false, &projectionMatrix[0][0]);
			glUniformMatrix4fv(m_GeometryShaderViewMatrixHandle, 1, false, &viewMatrix[0][0]);

			for (auto& renderCommand : renderCommands)
			{
				glUniformMatrix4fv(m_GeometryShaderModelMatrixHandle, 1, false, &renderCommand.modelMatrix[0][0]);
				glBindVertexArray(renderCommand.mesh->vertexArrayHandle);
				glDrawElements(GL_TRIANGLES, renderCommand.mesh->indexCount, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);
			}

			glDrawBuffer(GL_COLOR_ATTACHMENT3);
			glDisable(GL_DEPTH_TEST);

			glUseProgram(m_LightAccumulationShaderHandle);
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(m_LightAccumulationShaderGBufferTexture0Handle, 0);
			glBindTexture(GL_TEXTURE_2D, m_GeometryDiffuseTextureHandle);
			glActiveTexture(GL_TEXTURE1);
			glUniform1i(m_LightAccumulationShaderGBufferTexture1Handle, 1);
			glBindTexture(GL_TEXTURE_2D, m_GeometryNormalsTextureHandle);
			glActiveTexture(GL_TEXTURE2);
			glUniform1i(m_LightAccumulationShaderGBufferTexture2Handle, 2);
			glBindTexture(GL_TEXTURE_2D, m_GeometryPositionTextureHandle);
			glActiveTexture(GL_TEXTURE3);
			glUniform1i(m_LightAccumulationShaderDepthTextureHandle, 3);
			glBindTexture(GL_TEXTURE_2D, m_GeometryDepthTextureHandle);

			glUniformMatrix4fv(m_LightAccumulationShaderProjectionMatrixHandle, 1, false, &projectionMatrix[0][0]);
			glUniformMatrix4fv(m_LightAccumulationShaderViewMatrixHandle, 1, false, &viewMatrix[0][0]);
			glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
			glUniformMatrix4fv(m_LightAccumulationShaderViewProjectionMatrixHandle, 1, false, &viewProjectionMatrix[0][0]);
			glm::mat4 inverseProjectionMatrix = glm::inverse(projectionMatrix);
			glUniformMatrix4fv(m_LightAccumulationShaderInverseProjectionMatrixHandle, 1, false, &inverseProjectionMatrix[0][0]);
			glUniform2i(m_LightAccumulationShaderScreenSizeHandle, m_Window->GetWidth(), m_Window->GetHeight());

			glBindImageTexture(4, m_GeometryLightAccumulationTextureHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_LightsShaderStorageBufferHandle);
			glDispatchCompute(m_WorkGroupsX, m_WorkGroupsY, 1);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(m_CompositeShaderHandle);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(m_CompositeShaderGBufferTexture0Handle, 0);
		glBindTexture(GL_TEXTURE_2D, m_GeometryDiffuseTextureHandle);
		glActiveTexture(GL_TEXTURE1);
		glUniform1i(m_CompositeShaderGBufferTexture1Handle, 1);
		glBindTexture(GL_TEXTURE_2D, m_GeometryNormalsTextureHandle);
		glActiveTexture(GL_TEXTURE2);
		glUniform1i(m_CompositeShaderGBufferTexture2Handle, 2);
		glBindTexture(GL_TEXTURE_2D, m_GeometryPositionTextureHandle);
		glActiveTexture(GL_TEXTURE3);
		glUniform1i(m_CompositeShaderGBufferTexture3Handle, 3);
		glBindTexture(GL_TEXTURE_2D, m_GeometryLightAccumulationTextureHandle);
		glActiveTexture(GL_TEXTURE4);
		glUniform1i(m_CompositeShaderDepthTextureHandle, 4);
		glBindTexture(GL_TEXTURE_2D, m_GeometryDepthTextureHandle);

		glBindVertexArray(m_ScreenSpaceQuadVertexArrayHandle);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

}