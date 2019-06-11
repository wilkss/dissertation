#pragma once

#include "deliverable/renderer/TiledRenderer.h"

namespace deliverable {

	class ForwardPlusRenderer : public TiledRenderer
	{
	public:
		ForwardPlusRenderer(Window* window);
		~ForwardPlusRenderer();

		void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::vector<RenderCommand>& renderCommands, const std::vector<Light>& lights) override;

	private:
		unsigned m_DepthPrepassFbo;
		unsigned m_DepthPrepassDepthTexture;

		unsigned m_VisibleLightIndicesBuffer;

		unsigned m_LightCullingShaderHandle;
		unsigned m_LightCullingShaderDepthTextureHandle;
		unsigned m_LightCullingShaderProjectionMatrixHandle;
		unsigned m_LightCullingShaderViewMatrixHandle;
		unsigned m_LightCullingShaderViewProjectionMatrixHandle;
		unsigned m_LightCullingShaderScreenSizeHandle;
		unsigned m_LightCullingShaderInverseProjectionMatrixHandle;

		unsigned m_LightAccumulationShaderHandle;
		unsigned m_LightAccumulationShaderNumberOfTilesXHandle;
		unsigned m_LightAccumulationShaderProjectionMatrixHandle;
		unsigned m_LightAccumulationShaderViewMatrixHandle;
		unsigned m_LightAccumulationShaderModelMatrixHandle;

		unsigned m_EmptyShaderHandle;
		unsigned m_EmptyShaderProjectionMatrixHandle;
		unsigned m_EmptyShaderViewMatrixHandle;
		unsigned m_EmptyShaderModelMatrixHandle;

		unsigned m_VisibleIndicesShaderStorageBufferHandle;
	};

}