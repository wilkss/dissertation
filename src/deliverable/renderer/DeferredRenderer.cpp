#include "deliverable/renderer/DeferredRenderer.h"
#include "deliverable/Window.h"

#include <GL/glew.h>

namespace deliverable {

	DeferredRenderer::DeferredRenderer(Window* window, unsigned screenSpaceQuadVertexArrayHandle, unsigned lightSphereVertexArrayHandle, unsigned lightSphereIndexCount)
		: Renderer(window), m_ScreenSpaceQuadVertexArrayHandle(screenSpaceQuadVertexArrayHandle), m_LightSphereVertexArrayHandle(lightSphereVertexArrayHandle), m_LightSphereIndexCount(lightSphereIndexCount)
	{
		m_GeometryShaderHandle = CreateShaderProgram("media/shaders/Object.vertex", "media/shaders/deferred_pipeline/GBuffer.fragment");
		m_GeometryShaderProjectionMatrixHandle = glGetUniformLocation(m_GeometryShaderHandle, "u_ProjectionMatrix");
		m_GeometryShaderViewMatrixHandle = glGetUniformLocation(m_GeometryShaderHandle, "u_ViewMatrix");
		m_GeometryShaderModelMatrixHandle = glGetUniformLocation(m_GeometryShaderHandle, "u_ModelMatrix");

		m_EmptyShaderHandle = CreateShaderProgram("media/shaders/Object.vertex", "media/shaders/Empty.fragment");
		m_EmptyShaderProjectionMatrixHandle = glGetUniformLocation(m_EmptyShaderHandle, "u_ProjectionMatrix");
		m_EmptyShaderViewMatrixHandle = glGetUniformLocation(m_EmptyShaderHandle, "u_ViewMatrix");
		m_EmptyShaderModelMatrixHandle = glGetUniformLocation(m_EmptyShaderHandle, "u_ModelMatrix");

		m_LightShaderHandle = CreateShaderProgram("media/shaders/Object.vertex", "media/shaders/deferred_pipeline/Light.fragment");
		m_LightShaderGBufferTexture0Handle = glGetUniformLocation(m_LightShaderHandle, "u_GBufferTexture0");
		m_LightShaderGBufferTexture1Handle = glGetUniformLocation(m_LightShaderHandle, "u_GBufferTexture1");
		m_LightShaderGBufferTexture2Handle = glGetUniformLocation(m_LightShaderHandle, "u_GBufferTexture2");
		m_LightShaderDepthTextureHandle = glGetUniformLocation(m_LightShaderHandle, "u_DepthTexture");
		m_LightShaderProjectionMatrixHandle = glGetUniformLocation(m_LightShaderHandle, "u_ProjectionMatrix");
		m_LightShaderViewMatrixHandle = glGetUniformLocation(m_LightShaderHandle, "u_ViewMatrix");
		m_LightShaderModelMatrixHandle = glGetUniformLocation(m_LightShaderHandle, "u_ModelMatrix");
		m_LightShaderScreenSizeHandle = glGetUniformLocation(m_LightShaderHandle, "u_ScreenSize");
		m_LightShaderLightPositionAndRadiusHandle = glGetUniformLocation(m_LightShaderHandle, "u_Light.positionAndRadius");
		m_LightShaderLightColorHandle = glGetUniformLocation(m_LightShaderHandle, "u_Light.color");

		m_CompositeShaderHandle = CreateShaderProgram("media/shaders/ScreenSpaceQuad.vertex", "media/shaders/deferred_pipeline/Composite.fragment");
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

		glGenTextures(1, &m_GeometryBufferDepthStencilTextureHandle);
		glBindTexture(GL_TEXTURE_2D, m_GeometryBufferDepthStencilTextureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Window->GetWidth(), m_Window->GetHeight(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &m_GeometryBufferHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryBufferHandle);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_GeometryDiffuseTextureHandle, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_GeometryNormalsTextureHandle, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_GeometryPositionTextureHandle, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, m_GeometryLightAccumulationTextureHandle, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_GeometryBufferDepthStencilTextureHandle, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	DeferredRenderer::~DeferredRenderer()
	{
		glDeleteFramebuffers(1, &m_GeometryBufferHandle);
		glDeleteTextures(1, &m_GeometryDiffuseTextureHandle);
		glDeleteTextures(1, &m_GeometryNormalsTextureHandle);
		glDeleteTextures(1, &m_GeometryPositionTextureHandle);
		glDeleteTextures(1, &m_GeometryLightAccumulationTextureHandle);
		glDeleteTextures(1, &m_GeometryBufferDepthStencilTextureHandle);

		glDeleteProgram(m_GeometryShaderHandle);
		glDeleteProgram(m_EmptyShaderHandle);
		glDeleteProgram(m_LightShaderHandle);
		glDeleteProgram(m_CompositeShaderHandle);
	}

	void DeferredRenderer::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::vector<RenderCommand>& renderCommands, const std::vector<Light>& lights)
	{
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryBufferHandle);
		{
			glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

			glDrawBuffer(GL_NONE);
			glDepthMask(false);
			glCullFace(GL_FRONT);
			glDisable(GL_CULL_FACE);
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_ALWAYS, 0, 0);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);

			glUseProgram(m_EmptyShaderHandle);
			glUniformMatrix4fv(m_EmptyShaderProjectionMatrixHandle, 1, false, &projectionMatrix[0][0]);
			glUniformMatrix4fv(m_EmptyShaderViewMatrixHandle, 1, false, &viewMatrix[0][0]);

			for (auto& light : lights)
			{
				glm::mat4 modelMatrix(1.0f);
				modelMatrix = glm::translate(modelMatrix, glm::vec3(light.positionAndRadius.x, light.positionAndRadius.y, light.positionAndRadius.z));
				modelMatrix = glm::scale(modelMatrix, glm::vec3(light.positionAndRadius.w));
				glUniformMatrix4fv(m_EmptyShaderModelMatrixHandle, 1, false, &modelMatrix[0][0]);
				glBindVertexArray(m_LightSphereVertexArrayHandle);
				glDrawElements(GL_TRIANGLES, m_LightSphereIndexCount, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);
			}

			glDrawBuffer(GL_COLOR_ATTACHMENT3);
			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
			glDepthMask(true);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glUseProgram(m_LightShaderHandle);
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(m_LightShaderGBufferTexture0Handle, 0);
			glBindTexture(GL_TEXTURE_2D, m_GeometryDiffuseTextureHandle);
			glActiveTexture(GL_TEXTURE1);
			glUniform1i(m_LightShaderGBufferTexture1Handle, 1);
			glBindTexture(GL_TEXTURE_2D, m_GeometryNormalsTextureHandle);
			glActiveTexture(GL_TEXTURE2);
			glUniform1i(m_LightShaderGBufferTexture2Handle, 2);
			glBindTexture(GL_TEXTURE_2D, m_GeometryPositionTextureHandle);

			glUniformMatrix4fv(m_LightShaderProjectionMatrixHandle, 1, false, &projectionMatrix[0][0]);
			glUniformMatrix4fv(m_LightShaderViewMatrixHandle, 1, false, &viewMatrix[0][0]);
			glUniform2i(m_LightShaderScreenSizeHandle, m_Window->GetWidth(), m_Window->GetHeight());

			for (auto& light : lights)
			{
				glUniform4fv(m_LightShaderLightPositionAndRadiusHandle, 1, &light.positionAndRadius[0]);
				glUniform4fv(m_LightShaderLightColorHandle, 1, &light.color[0]);
				glm::mat4 modelMatrix(1.0f);
				modelMatrix = glm::translate(modelMatrix, glm::vec3(light.positionAndRadius.x, light.positionAndRadius.y, light.positionAndRadius.z));
				modelMatrix = glm::scale(modelMatrix, glm::vec3(light.positionAndRadius.w));
				glUniformMatrix4fv(m_LightShaderModelMatrixHandle, 1, false, &modelMatrix[0][0]);
				glBindVertexArray(m_LightSphereVertexArrayHandle);
				glDrawElements(GL_TRIANGLES, m_LightSphereIndexCount, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);
			}

			glDisable(GL_STENCIL_TEST);
			glCullFace(GL_BACK);
			glDisable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		glBindTexture(GL_TEXTURE_2D, m_GeometryBufferDepthStencilTextureHandle);

		glBindVertexArray(m_ScreenSpaceQuadVertexArrayHandle);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

}