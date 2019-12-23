#pragma once

#include <glm/glm.hpp>

namespace gd
{
	struct CanvasVertex
	{
		glm::vec2 Position;
		glm::vec2 UV;
		glm::vec4 Color;
	};
}