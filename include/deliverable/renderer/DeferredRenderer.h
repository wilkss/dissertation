#pragma once

#include "deliverable/renderer/Renderer.h"

namespace deliverable {

	class DeferredRenderer : public Renderer
	{
	public:
		DeferredRenderer(Window* window, unsigned screenSpaceQuadVertexArrayHandle, unsigned lightSphereVertexArrayHandle, unsigned lightSphereIndexCount);
		~DeferredRenderer();

		void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::vector<RenderCommand>& renderCommands, const std::vector<Light>& lights) override;

	private:
		unsigned m_GeometryBufferHandle;
		unsigned m_GeometryDiffuseTextureHandle;
		unsigned m_GeometryNormalsTextureHandle;
		unsigned m_GeometryPositionTextureHandle;
		unsigned m_GeometryLightAccumulationTextureHandle;
		unsigned m_GeometryBufferDepthStencilTextureHandle;

		unsigned m_GeometryShaderHandle;
		unsigned m_GeometryShaderProjectionMatrixHandle;
		unsigned m_GeometryShaderViewMatrixHandle;
		unsigned m_GeometryShaderModelMatrixHandle;

		unsigned m_EmptyShaderHandle;
		unsigned m_EmptyShaderProjectionMatrixHandle;
		unsigned m_EmptyShaderViewMatrixHandle;
		unsigned m_EmptyShaderModelMatrixHandle;

		unsigned m_LightShaderHandle;
		unsigned m_LightShaderGBufferTexture0Handle;
		unsigned m_LightShaderGBufferTexture1Handle;
		unsigned m_LightShaderGBufferTexture2Handle;
		unsigned m_LightShaderDepthTextureHandle;
		unsigned m_LightShaderProjectionMatrixHandle;
		unsigned m_LightShaderViewMatrixHandle;
		unsigned m_LightShaderModelMatrixHandle;
		unsigned m_LightShaderScreenSizeHandle;
		unsigned m_LightShaderLightPositionAndRadiusHandle;
		unsigned m_LightShaderLightColorHandle;

		unsigned m_CompositeShaderHandle;
		unsigned m_CompositeShaderGBufferTexture0Handle;
		unsigned m_CompositeShaderGBufferTexture1Handle;
		unsigned m_CompositeShaderGBufferTexture2Handle;
		unsigned m_CompositeShaderGBufferTexture3Handle;
		unsigned m_CompositeShaderDepthTextureHandle;

		unsigned m_LightSphereVertexArrayHandle;
		unsigned m_LightSphereIndexCount;

		unsigned m_ScreenSpaceQuadVertexArrayHandle;
	};

}