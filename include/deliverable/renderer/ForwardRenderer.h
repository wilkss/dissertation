#pragma once

#include "deliverable/renderer/Renderer.h"

namespace deliverable {

	class ForwardRenderer : public Renderer
	{
	public:
		ForwardRenderer(Window* window);
		~ForwardRenderer();

		void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const std::vector<RenderCommand>& renderCommands, const std::vector<Light>& lights) override;

	private:
		unsigned m_LightAccumulationShaderHandle;
		unsigned m_LightAccumulationShaderProjectionMatrixHandle;
		unsigned m_LightAccumulationShaderViewMatrixHandle;
		unsigned m_LightAccumulationShaderModelMatrixHandle;
	};

}