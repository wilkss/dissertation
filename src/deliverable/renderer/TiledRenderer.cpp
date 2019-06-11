#include "deliverable/renderer/TiledRenderer.h"
#include "deliverable/Window.h"

namespace deliverable {

	TiledRenderer::TiledRenderer(Window* window)
		: Renderer(window)
	{
		m_WorkGroupsX = (m_Window->GetWidth() + (m_Window->GetWidth() % 32)) / 32;
		m_WorkGroupsY = (m_Window->GetHeight() + (m_Window->GetHeight() % 32)) / 32;
	}

	TiledRenderer::~TiledRenderer()
	{
	}

}