#pragma once
#include "Settings.h"
#include "PipelineItem.h"

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

			void ShowProperties();
			void Compile();

		private:
			unsigned int m_shader, m_projMatrixLoc, m_modelMatrixLoc;
		};
	}
}