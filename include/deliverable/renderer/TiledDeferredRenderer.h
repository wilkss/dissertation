#pragma once

#include "deliverable/renderer/TiledRenderer.h"

namespace deliverable {

	class TiledDeferredRenderer : public TiledRenderer
	{
	public:
		TiledDeferredRenderer(Window* window, unsigned screenSpaceQuadVertexArrayHandle);
		~TiledDeferredRenderer();

		void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::vector<RenderCommand>& renderCommands, const std::vector<Light>& lights) override;

	private:
		unsigned m_GeometryBufferHandle;
		unsigned m_GeometryDiffuseTextureHandle;
		unsigned m_GeometryNormalsTextureHandle;
		unsigned m_GeometryPositionTextureHandle;
		unsigned m_GeometryLightAccumulationTextureHandle;
		unsigned m_GeometryDepthTextureHandle;

		unsigned m_GeometryShaderHandle;
		unsigned m_GeometryShaderProjectionMatrixHandle;
		unsigned m_GeometryShaderViewMatrixHandle;
		unsigned m_GeometryShaderModelMatrixHandle;
		unsigned m_GeometryShaderAlbedoHandle;

		unsigned m_LightAccumulationShaderHandle;
		unsigned m_LightAccumulationShaderGBufferTexture0Handle;
		unsigned m_LightAccumulationShaderGBufferTexture1Handle;
		unsigned m_LightAccumulationShaderGBufferTexture2Handle;
		unsigned m_LightAccumulationShaderDepthTextureHandle;
		unsigned m_LightAccumulationShaderProjectionMatrixHandle;
		unsigned m_LightAccumulationShaderViewMatrixHandle;
		unsigned m_LightAccumulationShaderViewProjectionMatrixHandle;
		unsigned m_LightAccumulationShaderScreenSizeHandle;
		unsigned m_LightAccumulationShaderInverseProjectionMatrixHandle;

		unsigned m_CompositeShaderHandle;
		unsigned m_CompositeShaderGBufferTexture0Handle;
		unsigned m_CompositeShaderGBufferTexture1Handle;
		unsigned m_CompositeShaderGBufferTexture2Handle;
		unsigned m_CompositeShaderGBufferTexture3Handle;
		unsigned m_CompositeShaderDepthTextureHandle;

		unsigned m_ScreenSpaceQuadVertexArrayHandle;
	};

}