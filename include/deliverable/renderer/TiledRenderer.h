#pragma once

#include "deliverable/renderer/Renderer.h"

#define MAX_LIGHTS_PER_TILE 1024

namespace deliverable {

	class TiledRenderer : public Renderer
	{
	public:
		TiledRenderer(Window* window);
		virtual ~TiledRenderer();

	protected:
		unsigned m_WorkGroupsX;
		unsigned m_WorkGroupsY;
	};

}