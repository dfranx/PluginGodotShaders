#pragma once
#include "Settings.h"
#include "PipelineItem.h"

#include <glm/glm.hpp>

namespace gd
{
	namespace pipe
	{
		class CanvasMaterial : public PipelineItem
		{
		public:
			char ShaderPath[MAX_PATH_LENGTH];

			CanvasMaterial();
			~CanvasMaterial();

			void SetViewportSize(float x, float y);
			void Bind();
			void ShowProperties();
			void Compile();

			inline void SetModelMatrix(glm::mat4 mat) { m_modelMat = mat; }

		private:
			unsigned int m_shader, m_projMatrixLoc, m_modelMatrixLoc;
			glm::mat4 m_projMat;
			glm::mat4 m_modelMat;
		};
	}
}